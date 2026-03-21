# there was no need for this
# run from root dir
import os

def read_lines_from_file(file_path):
    if not os.path.isfile(file_path):
        raise FileNotFoundError(f"The file {file_path} does not exist.")
    
    with open(file_path, 'r') as file:
        lines = file.readlines()
    
    return len(lines)

lines = []
total = 0
for file in os.listdir('.'):
    # can't be asked to make a list of extensions lol
    if file.endswith('.cpp') or file.endswith('.hpp') or file.endswith('.frag') or file.endswith('.vert') or file.endswith('.sh') or file.endswith('.txt')  or file.endswith('.py'):
        file_path = os.path.join('.', file)
        try:
            line_count = read_lines_from_file(file_path)
            lines.append((line_count, file_path))
            total += line_count
        except FileNotFoundError as e:
            print(e)

# get dah source files
for file in os.listdir('src'):
    # can't be asked to make a list of extensions lol
    if file.endswith('.cpp') or file.endswith('.hpp') or file.endswith('.frag') or file.endswith('.vert') or file.endswith('.json') or file.endswith('.py'):
        file_path = os.path.join('src', file)
        try:
            line_count = read_lines_from_file(file_path)
            lines.append((line_count, file_path))
            total += line_count
        except FileNotFoundError as e:
            print(e)

for file in os.listdir('src/core'):
    # can't be asked to make a list of extensions lol
    if file.endswith('.cpp') or file.endswith('.hpp') or file.endswith('.frag') or file.endswith('.vert') or file.endswith('.json') or file.endswith('.py'):
        file_path = os.path.join('src/core', file)
        try:
            line_count = read_lines_from_file(file_path)
            lines.append((line_count, file_path))
            total += line_count
        except FileNotFoundError as e:
            print(e)

for file in os.listdir('util'):
    # can't be asked to make a list of extensions lol
    if file.endswith('.cpp') or file.endswith('.hpp') or file.endswith('.frag') or file.endswith('.vert') or file.endswith('.py'):

        file_path = os.path.join('util', file)
        try:
            line_count = read_lines_from_file(file_path)
            lines.append((line_count, file_path))
            total += line_count
        except FileNotFoundError as e:
            print(e)

# and them shaders
for file in os.listdir('src/shaders'):
    if file.endswith('.cpp') or file.endswith('.hpp') or file.endswith('.frag') or file.endswith('.vert') or file.endswith('.json') or file.endswith('.py'):
        file_path = os.path.join('src/shaders', file)
        try:
            line_count = read_lines_from_file(file_path)
            lines.append((line_count, file_path))
            total += line_count
        except FileNotFoundError as e:
            print(e)

# don't forget tje builtin ones
for file in os.listdir('src/shaders/builtin'):
    if file.endswith('.cpp') or file.endswith('.hpp') or file.endswith('.frag') or file.endswith('.vert') or file.endswith('.json') or file.endswith('.py'):
        file_path = os.path.join('src/shaders/builtin', file)
        try:
            line_count = read_lines_from_file(file_path)
            lines.append((line_count, file_path))
            total += line_count
        except FileNotFoundError as e:
            print(e)

lines.sort(key=lambda x: x[0])
for line in lines:
    print(f"{line[1]}: {line[0]}")
print(f"Total lines: {total}")