import pyray as ray
import json

WIDTH, HEIGHT = 1800, 1200

class Editor:
    def __init__(self):
        ray.init_window(WIDTH, HEIGHT, "Level Editor")

        self.camera = ray.Camera3D()
        self.camera.position = ray.Vector3(10.0, 10.0, 10.0)
        self.camera.target = ray.Vector3(0.0, 0.0, 0.0)
        self.camera.up = ray.Vector3(0.0, 1.0, 0.0)
        self.camera.fovy = 45.0
        self.camera.projection = ray.CameraProjection.CAMERA_PERSPECTIVE

        self.assets = {
            "models": {}
        }
        self.objects = []

        self.load_level("data/maps/0.json")

        ray.set_config_flags(ray.ConfigFlags.FLAG_MSAA_4X_HINT | ray.ConfigFlags.FLAG_WINDOW_RESIZABLE | ray.ConfigFlags.FLAG_VSYNC_HINT)
        ray.disable_cursor()
        ray.set_target_fps(60)

    def load_level(self, path):
        with open(path, "r") as f:
            data = json.load(f)
            for key in data["level"]["models"]:
                self.assets["models"][key] = ray.load_model(data["level"]["models"][key]["path"])
            
            for object in data["level"]["objects"]:
                self.objects.append(object)
    
    def close(self):
        for key in self.assets["models"]:
            ray.unload_model(self.assets["models"][key])

        ray.close_window()

    def run(self):
        while not ray.window_should_close():
            ray.update_camera(self.camera, ray.CameraMode.CAMERA_FREE)
            
            ray.begin_drawing()
            ray.clear_background(ray.DARKBLUE)

            ray.begin_mode_3d(self.camera)

            for obj in self.objects:
                ray.draw_model_ex(self.assets["models"][str(obj["modelID"])],
                                  obj["position"], obj["rotation"], 0.0, obj["scale"], ray.WHITE)

            ray.draw_grid(10, 1.0)

            ray.end_mode_3d()

            ray.draw_fps(10, 400)

            ray.end_drawing()

if __name__ == "__main__":
    Editor().run()