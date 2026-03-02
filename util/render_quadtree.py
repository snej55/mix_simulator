import pygame, json

pygame.init()

WIDTH, HEIGHT = 1000, 1000                                                                                                                                                  

screen = pygame.display.set_mode((WIDTH, HEIGHT))
clock = pygame.time.Clock()
running = True

quads = []
entities = []
with open("util/quads.json", "r") as f:
    data = json.load(f)                 
    quads = data["quads"]
    entities = data["entities"]
scale = 4

scroll = [0, 0]

while running:
    scroll[0] += (int(pygame.key.get_pressed()[pygame.K_RIGHT]) - int(pygame.key.get_pressed()[pygame.K_LEFT])) * 20
    scroll[1] += (int(pygame.key.get_pressed()[pygame.K_DOWN]) - int(pygame.key.get_pressed()[pygame.K_UP])) * 20
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        elif event.type == pygame.KEYDOWN:
            if event.key == pygame.K_ESCAPE:
                running = False
    
    screen.fill((0, 0, 0))
    for quad in quads:

            pygame.draw.rect(screen, (255, 255, 255), (quad["pos"][0] * scale - scroll[0],
                                                        quad["pos"][1] * scale - scroll[1],
                                                        quad["dimensions"][0] * scale,
                                                            quad["dimensions"][1] * scale), 1)

    for entity in entities:
        pygame.draw.rect(screen, (255, 0, 0), (entity[0] * scale - scroll[0], entity[1] * scale - scroll[1],
                                               entity[2] * scale, entity[3] * scale))


    pygame.display.flip()
    clock.tick(60)

pygame.quit()