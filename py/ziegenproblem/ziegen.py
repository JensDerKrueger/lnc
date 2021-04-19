import random

noChangeWins = 0
changeWins = 0

while noChangeWins+changeWins < 10000:
	priceDoor  = random.randint(1,3)
	choiceDoor = random.randint(1,3)
	
	possibleOpenDoors = []
	if priceDoor != 1 and choiceDoor != 1:
		possibleOpenDoors += [1]
	if priceDoor != 2 and choiceDoor != 2:
		possibleOpenDoors += [2]
	if priceDoor != 3 and choiceDoor != 3:
		possibleOpenDoors += [3]
	openDoor = random.choice(possibleOpenDoors)

	if openDoor != 1 and choiceDoor != 1:
		changeDoor = 1
	if openDoor != 2 and choiceDoor != 2:
		changeDoor = 2
	if openDoor != 3 and choiceDoor != 3:
		changeDoor = 3
	
	print("The price is at door",priceDoor)
	print("The first choice is door",choiceDoor)
	print("Quizmaster opens door",openDoor)
	print("The alternative door is",changeDoor)
			
	if choiceDoor == priceDoor:
		noChangeWins += 1
		print("Staying with the original door wins.")
	if changeDoor == priceDoor:
		changeWins += 1
		print("Changing the door wins.")
		
	print("Change vs. no Change",changeWins, "/", noChangeWins)