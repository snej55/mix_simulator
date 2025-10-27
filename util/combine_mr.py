# a simple script to combine metallic and roughness textures for gltf
import pygame
import argparse
import os.path

# ---- get paths ---- #
parser = argparse.ArgumentParser("combine_mr")
parser.add_argument("metallic_path", help="Path to metallic map texture", type=str)
parser.add_argument("roughness_path", help="Path to roughness map texture", type=str)
args = parser.parse_args()

assert os.path.isfile(args.metallic_path), "Please enter a valid path to the metallic map texture!"
assert os.path.isfile(args.roughness_path), "Please enter a valid path to the roughness map texture!"

# ---- load images ---- #
pygame.init()
pygame.display.set_mode((0, 0))
metallic_map = pygame.image.load(args.metallic_path).convert()
roughness_map = pygame.image.load(args.roughness_path).convert()

# TODO: add support for interpolating pixel values for maps of different dimensions
assert metallic_map.size == roughness_map.size, "ERROR: Please make sure the maps have the same dimensions"

# ---- combine images ---- #

pygame.quit()
