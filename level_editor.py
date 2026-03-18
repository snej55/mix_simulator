import json, time

import pyray as ray

WIDTH, HEIGHT = 2000, 1900
POINT_LIGHT_SIZE = 1

class Editor:
    def __init__(self):
        ray.init_window(WIDTH, HEIGHT, "Level Editor")

        self.camera = ray.Camera3D()
        self.camera.position = ray.Vector3(10.0, 10.0, 10.0)
        self.camera.target = ray.Vector3(0.0, 0.0, 0.0)
        self.camera.up = ray.Vector3(0.0, 1.0, 0.0)
        self.camera.fovy = 45.0
        self.camera.projection = ray.CameraProjection.CAMERA_PERSPECTIVE

        self.assets = {"models": {}}
        self.body_types = ["static", "dynamic", "kinematic"]
        self.objects = []
        self.point_lights = []

        self.level_path = "data/maps/5.json"
        self.load_level(self.level_path)

        ray.set_config_flags(
            ray.ConfigFlags.FLAG_MSAA_4X_HINT
            | ray.ConfigFlags.FLAG_WINDOW_RESIZABLE
            | ray.ConfigFlags.FLAG_VSYNC_HINT
        )
        ray.disable_cursor()
        ray.set_target_fps(60)

        self.current_model_index = 0
        self.shift: bool = False
        self.dt = 1
        self.last_time = time.time() - 1 / 60

        self.controls = {
            "up": False,
            "down": False,
            "right": False,
            "left": False,
            "e": False,
            "q": False,
        }

        self.light_mode = False
        self.current_light_index = 0

    def progress_body_type(self):
        for i, body_type in enumerate(self.body_types):
            if (
                self.objects[self.current_model_index]["bodyType"].lower()
                == body_type.lower()
            ):
                print(self.objects[self.current_model_index]["bodyType"])
                self.objects[self.current_model_index]["bodyType"] = self.body_types[
                    (i + 1) % len(self.body_types)
                ]
                print("yo i switched")
                print(self.objects[self.current_model_index]["bodyType"])
                return

    # load level data
    def load_level(self, path):
        self.free_models()
        with open(path, "r") as f:
            data = json.load(f)
            for key in data["level"]["models"]:
                model_entry = data["level"]["models"][key]
                self.assets["models"][key] = {
                    "entry": model_entry,
                    "model": ray.load_model(data["level"]["models"][key]["path"]),
                }
            for object in data["level"]["objects"]:
                self.objects.append(object)
            for light in data["level"]["pointLights"]:
                self.point_lights.append(light)
            print(f"Loaded level data from `{path}`")

    def save_level(self, path):
        with open(path, "w") as f:
            data = {
                "level": {
                    "models": {
                        key: self.assets["models"][key]["entry"]
                        for key in self.assets["models"]
                    },
                    "objects": self.objects,
                    "pointLights": self.point_lights,
                }
            }
            json.dump(data, f, separators=(",", ":"))
            print(f"Saved level data to `{path}`")

    def free_models(self):
        for key in self.assets["models"]:
            ray.unload_model(self.assets["models"][key])
            del self.assets["models"][key]

    def close(self):
        self.free_models()
        ray.close_window()

    def handle_input(self):
        if ray.is_key_pressed(ray.KeyboardKey.KEY_X):
            if self.light_mode:
                if len(self.point_lights) > 0:
                    self.point_lights.pop(self.current_light_index)
                    self.current_light_index = self.current_light_index - 1
            else:
                if len(self.objects) > 0:
                    self.objects.pop(self.current_model_index)
                    self.current_model_index = self.current_model_index - 1
        if ray.is_key_pressed(ray.KeyboardKey.KEY_O):
            self.save_level(self.level_path)
        if ray.is_key_down(ray.KeyboardKey.KEY_LEFT_SHIFT) or ray.is_key_down(
            ray.KeyboardKey.KEY_RIGHT_SHIFT
        ):
            self.shift = True
        else:
            self.shift = False
        if ray.is_key_pressed(ray.KeyboardKey.KEY_TAB):
            if self.light_mode:
                if self.shift:
                    self.current_light_index = (self.current_light_index - 1) % len(
                        self.point_lights
                    )
                else:
                    self.current_light_index = (self.current_light_index + 1) % len(
                        self.point_lights
                    )
            else:
                if self.shift:
                    self.current_model_index = (self.current_model_index - 1) % len(
                        self.objects
                    )
                else:
                    self.current_model_index = (self.current_model_index + 1) % len(
                        self.objects
                    )
        if ray.is_key_pressed(ray.KeyboardKey.KEY_L) and self.shift:
            self.light_mode = not self.light_mode

        if ray.is_key_pressed(ray.KeyboardKey.KEY_N):
            if self.light_mode:
                if len(self.point_lights) < 32:
                    self.point_lights.append(
                        {"position": [0, 10, 0], "color": [1, 1, 1], "radius": 100}
                    )
                    self.current_light_index = len(self.point_lights) - 1
            else:
                self.objects.append(
                    {
                        "position": self.objects[self.current_model_index]["position"].copy(),
                        "rotation": [0, 0, 0],
                        "scale": [1, 1, 1],
                        "modelID": self.objects[self.current_model_index]["modelID"],
                        "animated": False,
                        "bodyType": "static",
                    }
                )
                self.current_model_index = len(self.objects) - 1

        if ray.is_key_pressed(ray.KeyboardKey.KEY_B):
            self.progress_body_type()
        if ray.is_key_pressed(ray.KeyboardKey.KEY_C):
            self.objects[self.current_model_index]["modelID"] = (
                self.objects[self.current_model_index]["modelID"] + 1
            ) % len(self.assets["models"])

        if not self.shift:
            self.controls["up"] = ray.is_key_down(ray.KeyboardKey.KEY_K)
            self.controls["down"] = ray.is_key_down(ray.KeyboardKey.KEY_J)
            self.controls["right"] = ray.is_key_down(ray.KeyboardKey.KEY_L)
            self.controls["left"] = ray.is_key_down(ray.KeyboardKey.KEY_H)
            self.controls["e"] = ray.is_key_down(ray.KeyboardKey.KEY_U)
            self.controls["q"] = ray.is_key_down(ray.KeyboardKey.KEY_I)

        speed = 0.5
        if self.light_mode:
            self.point_lights[self.current_light_index]["position"][0] += (
                (int(self.controls["right"]) - int(self.controls["left"]))
                * speed
                * self.dt
            )
            self.point_lights[self.current_light_index]["position"][1] += (
                (int(self.controls["up"]) - int(self.controls["down"]))
                * speed
                * self.dt
            )
            self.point_lights[self.current_light_index]["position"][2] += (
                (int(self.controls["e"]) - int(self.controls["q"])) * speed * self.dt
            )
        else:

            self.objects[self.current_model_index]["position"][0] += (
                (int(self.controls["right"]) - int(self.controls["left"]))
                * speed
                * self.dt
            )
            self.objects[self.current_model_index]["position"][1] += (
                (int(self.controls["up"]) - int(self.controls["down"]))
                * speed
                * self.dt
            )
            self.objects[self.current_model_index]["position"][2] += (
                (int(self.controls["e"]) - int(self.controls["q"])) * speed * self.dt
            )

        # speed = 3
        # movement = ray.Vector3(0, 0, 0)

        # if ray.is_key_down(ray.KeyboardKey.KEY_W): movement.x += speed
        # if ray.is_key_down(ray.KeyboardKey.KEY_S): movement.x -= speed
        # if ray.is_key_down(ray.KeyboardKey.KEY_D): movement.y += speed
        # if ray.is_key_down(ray.KeyboardKey.KEY_A): movement.y -= speed
        # if ray.is_key_down(ray.KeyboardKey.KEY_E): movement.z += speed
        # if ray.is_key_down(ray.KeyboardKey.KEY_Q): movement.z -= speed

        # mouse_delta = ray.get_mouse_delta()
        # rotation = ray.Vector3(
        #     mouse_delta.x * 0.05,
        #     mouse_delta.y * 0.05,
        #     0.0
        # )

        # ray.update_camera_pro(self.camera, movement, rotation, 0.0)

    def update(self):
        self.handle_input()
        move_speed = 0.5
        rotation_speed = 0.05
        
        if ray.is_key_down(ray.KeyboardKey.KEY_LEFT_CONTROL):
            move_speed *= 3.0

        movement = ray.Vector3(
            (ray.is_key_down(ray.KeyboardKey.KEY_W) - ray.is_key_down(ray.KeyboardKey.KEY_S)) * move_speed,
            (ray.is_key_down(ray.KeyboardKey.KEY_D) - ray.is_key_down(ray.KeyboardKey.KEY_A)) * move_speed,
            (ray.is_key_down(ray.KeyboardKey.KEY_E) - ray.is_key_down(ray.KeyboardKey.KEY_Q)) * move_speed
        )

        mouse_delta = ray.get_mouse_delta()
        rotation = ray.Vector3(
            mouse_delta.x * rotation_speed,
            mouse_delta.y * rotation_speed,
            0.0
        )

        ray.update_camera_pro(self.camera, movement, rotation, 0.0)

    def run(self):
        while not ray.window_should_close():
            self.dt = (time.time() - self.last_time) * 60
            self.last_time = time.time()
            self.update()

            ray.begin_drawing()
            ray.clear_background(ray.DARKBLUE)

            ray.begin_mode_3d(self.camera)

            for i, obj in enumerate(self.objects):
                current_model: bool = i == self.current_model_index
                color = ray.WHITE
                if (current_model) and not self.light_mode:
                    color = ray.BLUE

                ray.draw_model_ex(
                    self.assets["models"][str(obj["modelID"])]["model"],
                    obj["position"],
                    [
                        obj["rotation"][0],
                        obj["rotation"][1],
                        obj["rotation"][2],
                    ],
                    360,
                    obj["scale"],
                    color,
                )

            for i, light in enumerate(self.point_lights):
                color = (
                    int(light["color"][0] * 255),
                    int(light["color"][1] * 255),
                    int(light["color"][2] * 255),
                )
                if i == self.current_light_index and self.light_mode:
                    color = (0, 0, 0)
                ray.draw_cube(
                    light["position"],
                    POINT_LIGHT_SIZE,
                    POINT_LIGHT_SIZE,
                    POINT_LIGHT_SIZE,
                    color,
                )

            # ray.draw_grid(200, 1.0)

            ray.end_mode_3d()

            ray.draw_fps(10, 10)
            ray.draw_text(
                f"CURRENT_MODEL_INDEX: {self.current_model_index}",
                10,
                28,
                16,
                ray.WHITE,
            )
            ray.draw_text(
                f"PHYSICS_BODY_TYPE: {self.objects[self.current_model_index]["bodyType"]}",
                10,
                46,
                16,
                ray.WHITE,
            )
            ray.draw_text(
                f"OBJECT_POSITION: {self.objects[self.current_model_index]["position"][0] :.1f}, {self.objects[self.current_model_index]["position"][1] : .1f}, {self.objects[self.current_model_index]["position"][2] :.1f}",
                10,
                64,
                16,
                ray.WHITE,
            )
            ray.draw_text(
                f"OBJECT_ROTATION: {self.objects[self.current_model_index]["rotation"][0] :.1f}, {self.objects[self.current_model_index]["rotation"][1] : .1f}, {self.objects[self.current_model_index]["rotation"][2] :.1f}",
                10,
                82,
                16,
                ray.WHITE,
            )
            ray.draw_text(
                f"OBJECT_SCALE: {self.objects[self.current_model_index]["scale"][0] :.1f}, {self.objects[self.current_model_index]["scale"][1] : .1f}, {self.objects[self.current_model_index]["scale"][2] :.1f}",
                10,
                100,
                16,
                ray.WHITE,
            )
            ray.draw_text(
                f"ANIMATED: {self.objects[self.current_model_index]["animated"]}",
                10,
                118,
                16,
                ray.WHITE,
            )
            ray.draw_text(
                f"MODEL_ID: {self.objects[self.current_model_index]["modelID"]}",
                10,
                136,
                16,
                ray.WHITE,
            )
            ray.draw_text(f"LIGHT_MODE: {self.light_mode}", 10, 154, 16, ray.WHITE)

            ray.end_drawing()


if __name__ == "__main__":
    Editor().run()
