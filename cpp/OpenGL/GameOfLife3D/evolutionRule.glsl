uniform mat4 evolutionParameters;

int evolutionRule(int center, int n) {
    int result;
    int deathRanges[8] = int[8]( int(evolutionParameters[0][0]), int(evolutionParameters[0][1]),
                                 int(evolutionParameters[0][2]), int(evolutionParameters[0][3]),
                                 int(evolutionParameters[1][0]), int(evolutionParameters[1][1]),
                                 int(evolutionParameters[1][2]), int(evolutionParameters[1][3]));
    int birthRanges[8] = int[8]( int(evolutionParameters[2][0]), int(evolutionParameters[2][1]),
                                 int(evolutionParameters[2][2]), int(evolutionParameters[2][3]),
                                 int(evolutionParameters[3][0]), int(evolutionParameters[3][1]),
                                 int(evolutionParameters[3][2]), int(evolutionParameters[3][3]));

    result = center;
    if (center == 1) {
        for (int i = 0; i < deathRanges.length(); i += 2) {
            if (n >= deathRanges[i] && n <= deathRanges[i + 1]) result = 0;
        }
    } else {
        for (int i = 0; i < birthRanges.length(); i += 2) {
            if (n >= birthRanges[i] && n <= birthRanges[i + 1]) result = 1;
        }
    }
    return result;
}



int evolutionRule0(int center, int n) {
  int result;
  if (center == 1) {
    result = n == 2 || n == 3 ? 1 : 0;
  } else {
    result = n == 3 ? 1 : 0;
  }
  return result;
}


//no blob, weak population, small stabil configs, rare glider (seed-density: 10%)
int evolutionRule1(int center, int n) {
    int result;
    const int deathRanges[6] = int[6]( 0, 3, 6, 9, 10, 27 );
    const int birthRanges[4] = int[4]( 5, 5, 12, 13 );

    result = center;    
    if (center == 1) {
        for (int i = 0; i < deathRanges.length(); i += 2) {
            if (n >= deathRanges[i] && n <= deathRanges[i + 1]) result = 0;
        }
    } else {
        for (int i = 0; i < birthRanges.length(); i += 2) {
            if (n >= birthRanges[i] && n <= birthRanges[i + 1]) result = 1;
        }
    }
    return result;
}


//rare blob, small stabil configs (seed-density: 10%)
int evolutionRule2(int center, int n) {
    int result;
    const int deathRanges[6] = int[6](0, 4, 6, 6, 10, 27);
    const int birthRanges[4] = int[4](5, 5, 12, 13);

    result = center;    
    if (center == 1) {
        for (int i = 0; i < deathRanges.length(); i += 2) {
            if (n >= deathRanges[i] && n <= deathRanges[i + 1]) result = 0;
        }
    }
    else {
        for (int i = 0; i < birthRanges.length(); i += 2) {
            if (n >= birthRanges[i] && n <= birthRanges[i + 1]) result = 1;
        }
    }
    return result;
}
