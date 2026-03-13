import bpy
import os

raw_path = "~/projects/mix_simulator/data/models/mug_shards/"
export_path = os.path.expanduser(raw_path)

if not os.path.exists(export_path):
    os.makedirs(export_path)

for i, obj in enumerate(bpy.context.scene.objects):
    if obj.type == 'MESH':
        bpy.ops.object.select_all(action='DESELECT')
        obj.select_set(True)
        
        safe_name = bpy.path.clean_name(obj.name)
        filename = os.path.join(export_path, str(i) + ".glb")
        
        bpy.ops.export_scene.gltf(filepath=filename, export_format='GLB', use_selection=True)
        print(f"Exported: {filename}")
