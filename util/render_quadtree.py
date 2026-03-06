import pygame, json

pygame.init()

WIDTH, HEIGHT = 2200, 1300                                                                                                                                                  

screen = pygame.display.set_mode((WIDTH, HEIGHT))
clock = pygame.time.Clock()
running = True

quads = []
entities = []
with open("util/quads.json", "r") as f:
    data = json.load(f)                 
    quads = data["quads"]
    entities = data["entities"]
scale = 8

scroll = [0, 0]

current_node_index = 0

while running:
    scroll[0] += (int(pygame.key.get_pressed()[pygame.K_RIGHT]) - int(pygame.key.get_pressed()[pygame.K_LEFT])) * 20
    scroll[1] += (int(pygame.key.get_pressed()[pygame.K_DOWN]) - int(pygame.key.get_pressed()[pygame.K_UP])) * 20
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        elif event.type == pygame.KEYDOWN:
            if event.key == pygame.K_ESCAPE:
                running = False
            if event.key == pygame.K_n:
                current_node_index = (current_node_index + 1) % len(quads)
    
    screen.fill((0, 0, 0))
    for entity in entities:
        pygame.draw.rect(screen, (255, 0, 0), (entity[0] * scale - scroll[0], entity[1] * scale - scroll[1],
                                               entity[2] * scale, entity[3] * scale))
    for quad in quads:
        if quad["solid"]:
            pygame.draw.rect(screen, (100, 0, 255), (quad["pos"][0] * scale - scroll[0],
                                                        quad["pos"][1] * scale - scroll[1],
                                                        quad["dimensions"][0] * scale,
                                                            quad["dimensions"][1] * scale), 1)
        else:
            pygame.draw.rect(screen, (255, 255, 255), (quad["pos"][0] * scale - scroll[0],
                                                        quad["pos"][1] * scale - scroll[1],
                                                        quad["dimensions"][0] * scale,
                                                            quad["dimensions"][1] * scale), 1)
            if not quad["hasChildren"]:
                dist = 4
                color = pygame.Color(255, 10, 10).lerp(pygame.Color(0, 200, 0).lerp(pygame.Color(0, 100, 100), min(1, max(quad["cost"] - dist, 0) / dist)), min(1, quad["cost"] / dist))
                pygame.draw.rect(screen, color, (quad["pos"][0] * scale - scroll[0] + 1,
                                                        quad["pos"][1] * scale - scroll[1] + 1,
                                                        quad["dimensions"][0] * scale - 2,
                                                            quad["dimensions"][1] * scale - 2))

    for quad in quads:
        if not quad["hasChildren"] and not quad["solid"]:
            direction_scale = 0.7
            pygame.draw.line(screen, (255, 255, 255), ((quad["pos"][0] + quad["dimensions"][0] * 0.5) * scale - scroll[0],
                                                   (quad["pos"][1] + quad["dimensions"][1] * 0.5) * scale - scroll[1]), ((quad["pos"][0] + quad["dimensions"][0] * 0.5 - quad["direction"][0] * direction_scale) * scale - scroll[0],
                                                   (quad["pos"][1] + quad["dimensions"][1] * 0.5 - quad["direction"][1] * direction_scale) * scale - scroll[1]))
            pygame.draw.circle(screen, (255, 255, 255), ((quad["pos"][0] + quad["dimensions"][0] * 0.5 - quad["direction"][0] * direction_scale) * scale - scroll[0],
                                                   (quad["pos"][1] + quad["dimensions"][1] * 0.5 - quad["direction"][1] * direction_scale) * scale - scroll[1]), 2)

    current_quad = quads[current_node_index]
    if not current_quad["hasChildren"] and not current_quad["solid"]:
        pygame.draw.rect(screen, (0, 255, 0), (current_quad["pos"][0] * scale - scroll[0],
                                                            current_quad["pos"][1] * scale - scroll[1],
                                                            current_quad["dimensions"][0] * scale,
                                                                current_quad["dimensions"][1] * scale))
        for i in current_quad["neighbours"]:
            quad = quads[i]
            pygame.draw.rect(screen, (255, 255, 20), (quad["pos"][0] * scale - scroll[0],
                                                            quad["pos"][1] * scale - scroll[1],
                                                            quad["dimensions"][0] * scale,
                                                                quad["dimensions"][1] * scale))
        quad = quads[current_quad["target"]]
        pygame.draw.rect(screen, (255, 0, 0), (quad["pos"][0] * scale - scroll[0],
                                                            quad["pos"][1] * scale - scroll[1],
                                                            quad["dimensions"][0] * scale,
                                                                quad["dimensions"][1] * scale))

    pygame.display.flip()
    clock.tick(60)

pygame.quit()