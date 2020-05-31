import sys

class Person:
  def __init__(self, vn, nn):
    self.vn = vn
    self.nn = nn

  def vorname(self):
    return self.vn

  def nachname(self):
    return self.nn

class Student(Person):
  def __init__(self, vn, nn, mat):
    super().__init__(vn,nn)
    self.mat = mat

  def nummer(self):
    return str(self.mat)

if __name__ == "__main__":
  p = Student("Jens", "Krüger", 214446)
  sys.stdout.write(p.nummer()+"\n")
