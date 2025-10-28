# a simple script to combine metallic and roughness textures for gltf
import pygame
import argparse
import os.path
import numpy as np

# ---- get paths ---- #
parser = argparse.ArgumentParser("combine_mr")
parser.add_argument("metallic_path", help="Path to metallic map texture", type=str)
parser.add_argument("roughness_path", help="Path to roughness map texture", type=str)
args = parser.parse_args()

assert os.path.isfile(args.metallic_path), "Please enter a valid path to the metallic map texture!"
assert os.path.isfile(args.roughness_path), "Please enter a valid path to the roughness map texture!"

# ---- load images ---- #
pygame.init()
display = pygame.display.set_mode((500, 500))
metallic_map = pygame.image.load(args.metallic_path).convert()
roughness_map = pygame.image.load(args.roughness_path).convert()

# TODO: add support for interpolating pixel values for maps of different dimensions
assert metallic_map.size == roughness_map.size, "ERROR: Please make sure the maps have the same dimensions"

# ---- combine images ---- #
combined = pygame.Surface(metallic_map.size)
combined.fill((0, 0, 0))
combined_pxarray = pygame.PixelArray(combined)
metalllic_pxarray = pygame.PixelArray(metallic_map)
roughness_pxarray = pygame.PixelArray(roughness_map)

print(f"Metallic byte size: {metalllic_pxarray.itemsize}")   
print(f"Roughness byte size: {roughness_pxarray.itemsize}")

for y in range(metallic_map.height):
    for x in range(metallic_map.width):
        combined_pxarray[x, y] = combined.map_rgb((
            255,
            roughness_map.unmap_rgb(roughness_pxarray[x, y]).r,
            metallic_map.unmap_rgb(metalllic_pxarray[x, y]).r,
        ))

# close pixel arrays
combined_pxarray.close()
metalllic_pxarray.close()
roughness_pxarray.close()

# ---- display result ---- ]
running = True
clock = pygame.time.Clock()
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        elif event.type == pygame.KEYDOWN:
            if event.key == pygame.K_ESCAPE:
                running = False
    
    display.fill((0, 0, 0))
    display.blit(pygame.transform.scale(combined, (450, 450)), (25, 25))

    pygame.display.flip()
    clock.tick()
            
pygame.quit()
