#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
	Allows you to read the packaging information of a food product
	in order to throw it in the right garbage can.
	Alex Fabre 19.05.2019
"""

import urllib.request
import json
import time

def get_packaging_info(product_code):
	"""
		Gets the packaging informations of the product from its
		product_code (barcode or RFID tag), on OpenFoodFacts database
	"""
	check_code(product_code)

	# Get informations from openfoodfacts.org
	url = "https://fr.openfoodfacts.org/api/v0/produit/"+ str(product_code) +".json"
	response = urllib.request.urlopen(url)
	data = json.loads(response.read())
	
	# Verify that the product was found
	if data['status'] != 1:
		print("Warning: " + str(data['status_verbose']))
		exit() # TODO: Ask user when prooduct is not found
	# Return packaging informations
	packaging = data['product']['packaging']

	if packaging == "":
		print("Warning: No packaging informations")

	return packaging


def check_code(product_code):
	"""
		Verify that the product code is a correct integer > 0
	"""
	if product_code <= 0:
		print("Error: Product code not positive integer : "+ str(product_code))
		exit()
	else:
		return 0


def is_recyclable(packaging):
	"""
		Verify that the packaging informations of the product contains
		any of the words listed in keywords.txt. If so, the product goes
		into recycle bin.
	"""
	recycle = 0
	
	with open("./keywords.txt", "r") as keywords: 
		for item in keywords:
			if item.strip() in packaging.lower():
				recycle +=1

	if recycle:
		print("Product goes recycling")
		return True
	else:
		print("Product goes to trash")
		return False




if __name__ == '__main__':

	start_time = time.time()

############## Usage exemple ###################################################

	product_code = 3029330003533

	packaging = get_packaging_info(product_code)

	if is_recyclable(packaging):
		# Open recyclable bin
		pass
	else:
		# Open trash bin
		pass

################################################################################	
	
	print("--- %s seconds ---" % (time.time() - start_time))
