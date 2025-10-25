# Created by Jens Kromdijk
# Simple script to create default shader files and add them to shaders.json
import json

# vertex shader
vert = """#version 410 core
layout (location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos, 1.0);
}
"""

# fragment shader
frag = """#version 410 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0);
}
"""


# save shader files and add shader to shaders.json
def save(shader_name: str, builtin: bool = False) -> None:
    # load shaders.json and check if shader exists
    shader_type = "custom"
    if builtin:
        shader_type = "builtin"
    shaders = {}
    with open("shaders.json", "r") as f:
        shaders = json.load(f)
        for shader in shaders[shader_type]:
            if shader["name"].lower() == shader_name.lower():
                print(f"ERROR: Custom shader `{shader_name}` already exists:")
                print(shader)
                return

    # save vertex shader
    vert_path = shader_name + ".vert"
    frag_path = shader_name + ".frag"

    if builtin:
        vert_path = "builtin/" + vert_path
        frag_path = "builtin/" + frag_path

    with open(vert_path, "w") as f:
        f.write(vert)
    # same for fragment shader
    with open(frag_path, "w") as f:
        f.write(frag)

    print(f"Created shader files: `{vert_path}` `{frag_path}`")

    # add shader to shaders.json
    with open("shaders.json", "w") as f:
        shaders[shader_type].append(
            {"name": shader_name, "shader": {"vert": vert_path, "frag": frag_path}}
        )
        json.dump(shaders, f, indent=4, sort_keys=True)

    print(f"Added shader `{shader_name}` to shaders.json")


# input shader
if __name__ == "__main__":
    name = input("Enter shader name: ")
    if name != "":
        save(name)
