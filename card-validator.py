class Card:
	def __init__(self, number):
		self.number = number
		self.c_type = self.determine_card()
		self.valid_length = self.check_length()
		self.passes_Luhn = self.Luhn()
		self.valid = self.check_valid()

	def determine_card(self):
		#Check starting numbers for card type
		if self.number[0] == "4":
			return "Visa"
		elif int(self.number[0:2:1]) >= 51 and int(self.number[0:2:1]) <= 55:
			return "Mastercard"
		elif self.number[0:2:1] == "34" or self.number[0:2:1] == "37":
			return "AMEX"
		elif self.number[0:4:1] == "6011":
			return "Discover"
		else:
			return "Invalid"

	def check_length(self):
		#First check for valid card type
		if self.c_type != "Invalid": 
			#Check length for different card types
			if self.c_type == "AMEX" and len(self.number) == 15:
				return True
			elif len(self.number) == 16:
				return True
			else:
				return False
		else:
			return False

	def Luhn(self):
		total = 0
		new_num = list(self.number)

		#Double every other digit from the second to last
		for x in range(-2, -len(self.number)-1, -2): 
			new_num[x] = int(self.number[x])*2

		#Add up the digits
		for x in new_num: 
			if(len(x) == 2):
				total += int(x[0]) + int(x[1])
			else:
				total += int(x)

		#Check if evenly divisible by 10 and return
		if total % 10 == 0:
			return True
		else:
			return False

	def check_valid(self):
		#If card meets all requirements return true
		if self.c_type != "Invalid" and self.valid_length and self.passes_Luhn:
			return True
		else:
			return False

def is_valid_card(card):
	if card.valid:
		print("This is a valid {} card.".format(card.c_type))
	elif card.c_type == "Invalid":
		print("This is not a valid card number")
	else:
		print("This is not a valid {} card".format(card.c_type))


cardx = Card(input("Input Card Number: "))
is_valid_card(cardx)