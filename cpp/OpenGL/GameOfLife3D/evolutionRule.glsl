float evolutionRule(float center, float n) {
  float result;
  if (center == 1) {
    result = n == 2.0 || n == 3.0 ? 1.0 : 0.0;
  } else {
    result = n == 3.0 ? 1.0 : 0.0;
  }
  return result;
}
