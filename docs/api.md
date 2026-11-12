# API Contract 
## Common Rules

* Always include units in field names(`_m, _c, _kmh, _min, _pct`)

* Timestamps use ISO 8601 (`2026-11-14T09:00`) in local Greece time(`Europe/Athens`)

* Errors always return {`"error": "..."`} : `400` for invalid input, `502` when Open_Meteo is unreachable.

* Breaking changes require a new path (`/api/v2/...`), never silent changes.

## Weather Service (`:8081`)

### GET /api/health
Response(`200 OK`)

`{"status": "ok"}`

### GET /api/conditions
Query parameters: `lat, lon, elevation, time`

Example: `GET /api/conditions?
lat=37.94&lon=23.82&elevation=1026&time=2026-11-14T09:00`

Response(`200 OK`):
{
  "location": { "lat": 37.94, "lon": 23.82, "elevation_m": 1026 },
  "time": "2026-11-14T09:00",
  "temperature_c": 8.4,
  "feels_like_c": 3.1,
  "wind_kmh": 22,
  "gusts_kmh": 41,
  "precip_probability_pct": 20,
  "sunset": "2026-11-14T17:12",
  "daylight_left_min": 492,
  "verdict": "CAUTION",
  "reasons": ["Gusts above 40 km/h"],
  "source": "Open-Meteo"
}

### POST /api/assess
Request body:
{
  "start_time": "2026-11-14T09:00",
  "duration_min": 240,
  "points": [
    { "lat": 37.97, "lon": 23.80, "elevation_m": 420 },
    { "lat": 37.94, "lon": 23.82, "elevation_m": 1026 }
  ]
}

Response(`200 OK`):
{
  "verdict": "CAUTION",
  "finishes_before_sunset": true,
  "sunset": "2026-11-14T17:12",
  "margin_min": 252,
  "reasons": ["Gusts above 40 km/h"]
}