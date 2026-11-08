#include<iostream>
#include"Grid.hpp"
#include<stdexcept>
#include<iomanip>
using namespace std;


void printgrid(const Grid& g){
    for(int r =0; r < g.rows();r++) { 
        for(int c = 0; c <g.cols(); c++){
            std::cout << std::setw(6) << g.at(r, c); 
        }
        cout<<'\n';
    }
}

void fill(Grid& g) { 
    //h=10r+c
    for(int i = 0; i <g.rows();i++){
        for(int j = 0; j<g.cols();j++){
            g.set(i,j,100+15*j);
        }
    }
}
int main() { 
    Grid g(3,4,30.0);
     // 3. Γραμμή 3 σε πλέγμα με 3 γραμμές (0, 1, 2)
    try {
        g.set(3, 0, 50.0);
        cout << "BUG: set(3, 0) did not throw\n";
    } catch (const std::out_of_range& e) {
        cout << "OK  set(3, 0):     " << e.what() << '\n';
    }

    // 4. Δύο αρνητικές διαστάσεις (το γινόμενο θα ήταν θετικό!)
    try {
        Grid bad(-2, -3, 30.0);
        cout << "BUG: Grid(-2, -3, 30) did not throw\n";
    } catch (const invalid_argument& e) {
        cout << "OK  Grid(-2,-3,30): " << e.what() << '\n';
    }

    // 5. Μηδενικό cellsize
    try {
        Grid bad(3, 4, 0.0);
        cout << "BUG: Grid(3, 4, 0) did not throw\n";
    } catch (const invalid_argument& e) {
        cout << "OK  Grid(3,4,0):    " << e.what() << '\n';
    }
    fill(g);
    printgrid(g);
    return 0;

}