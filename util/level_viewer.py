import pygame, sys, time, json

pygame.init()
pygame.mixer.init()

WIDTH, HEIGHT = 640, 480
SCALE = 2

class Editor:
    def __init__(self):
        self.display = pygame.display.set_mode((WIDTH, HEIGHT))
        self.screen = pygame.Surface((WIDTH // SCALE, HEIGHT // SCALE))
        self.clock = pygame.time.Clock()
        self.running = True

        self.dt = 1
        self.last_time = time.time() - 1/60

        self.objects: list[dict] = []
    
    def load_level_data(self, level_path):
        with open(level_path, "r") as f:
            data = json.load(f)
    
    def close(self):
        self.running = False
        pygame.quit()
        sys.exit()

    def update(self):
        # update deltatime
        self.dt = time.time() - self.last_time
        self.dt *= 60
        self.last_time = time.time()

        # update rest of game state 
        pass

    def draw(self):
        self.screen.fill((0, 0, 0))
        # draw stuff
        pass
    
    def run(self):
        while self.running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    self.close()
                if event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_ESCAPE:
                        self.close()

            self.update()
            self.draw()

            pygame.transform.scale_by(self.screen, SCALE, self.display)
            pygame.display.set_caption(f"FPS: {self.clock.get_fps() :.1f}")
            pygame.display.flip()
            self.clock.tick()

if __name__ == "__main__":
    Editor().run()
