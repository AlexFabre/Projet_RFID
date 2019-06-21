# Projet RFID
Poubelle connectée

## Descritpion

Le script python *packaging.py* permet de venir récupérer les informations de packaging d'un produit lorsqu'il est détécté dans la poubelle.

## Utilisation

Charger le module dans votre programme principal: *import packaging*

Trois fonctions sont mises à disposition:

- get_packaging_info(product_code) : Permet de récupérer sous la forme d'une chaine de caractères la description du packaging du produit.

- is_recyclable(packaging) : Permet de déterminer si le produit est recyclable en fonction des mots contenu dans la description du packaging et d'une liste de mots clefs prédéfinis dans le fichier *keywords.txt*

### Exemple d'utilisation

```python

packaging = get_packaging_info(product_code)

if is_recyclable(packaging):
	# Open recyclable bin
	pass
else:
	# Open trash bin
	pass

```

