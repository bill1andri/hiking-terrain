#include "DemLoader.hpp"

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// ---------------- DemTile ----------------

DemTile::DemTile(int n, double latSouth, double lonWest, std::vector<std::int16_t> data)
    : _size(n), _latSouth(latSouth), _lonWest(lonWest), _data(std::move(data)) {
    if (n <= 0) {
        throw std::invalid_argument("DemTile: size must be positive, got " +
                                    std::to_string(n));
    }
    // μετατροπή ΠΡΙΝ τον πολλαπλασιασμό, ώστε να μη γίνει overflow σε int
    const std::size_t expected =
        static_cast<std::size_t>(_size) * static_cast<std::size_t>(_size);
    if (_data.size() != expected) {  // το _data, όχι το data (έχει μετακινηθεί)
        throw std::invalid_argument("DemTile: expected " + std::to_string(expected) +
                                    " values, got " + std::to_string(_data.size()));
    }
}

std::int16_t DemTile::at(int r, int c) const {
    if (r < 0 || r >= _size || c < 0 || c >= _size) {
        throw std::out_of_range("DemTile: (r, c) out of range");
    }
    return _data[static_cast<std::size_t>(r) * _size + c];
}

// ---------------- loadHgt ----------------

static constexpr long kMinSize = 2;
static constexpr long kMaxSize = 3601;  // SRTM 1 arc-second

DemTile loadHgt(const std::string& path, double latSouth, double lonWest) {
    // α. Μέγεθος αρχείου, χωρίς να το ανοίξουμε
    std::uintmax_t bytes = 0;
    try {
        bytes = std::filesystem::file_size(path);
    } catch (const std::exception&) {
        throw std::runtime_error("cannot read file: " + path);
    }

    // β. n από size = 2 * n * n
    const long n = std::lround(std::sqrt(static_cast<double>(bytes) / 2.0));

    // γ. Έλεγχος ΠΡΙΝ δεσμεύσουμε μνήμη (υπολογισμός σε uintmax_t, χωρίς overflow)
    const bool exactSquare =
        n >= 1 && static_cast<std::uintmax_t>(n) * static_cast<std::uintmax_t>(n) * 2 == bytes;
    if (!exactSquare || n < kMinSize || n > kMaxSize) {
        throw std::runtime_error("not a valid .hgt file (size " + std::to_string(bytes) +
                                 " bytes): " + path);
    }

    // δ. Διάβασμα όλου του αρχείου με ΜΙΑ ανάγνωση
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("cannot open file: " + path);
    }
    std::vector<unsigned char> raw(static_cast<std::size_t>(bytes));
    if (!in.read(reinterpret_cast<char*>(raw.data()),
                 static_cast<std::streamsize>(raw.size()))) {
        throw std::runtime_error("could not read all bytes of: " + path);
    }

    // ε. Big-endian -> int16 (πρώτο byte = μεγάλο)
    const std::size_t count = static_cast<std::size_t>(n) * static_cast<std::size_t>(n);
    std::vector<std::int16_t> data(count);
    for (std::size_t i = 0; i < count; i++) {
        data[i] = static_cast<std::int16_t>((raw[2 * i] << 8) | raw[2 * i + 1]);
    }

    // στ. Επιστροφή, ΜΕΤΑΦΕΡΟΝΤΑΣ τα δεδομένα (καμία αντιγραφή)
    return DemTile(static_cast<int>(n), latSouth, lonWest, std::move(data));
}   