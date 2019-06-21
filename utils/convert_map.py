import re
import sys

char_to_entity = {
    'm': '*',
    's': 'O',
    'w': '#',
    'g': '+',
    'b': ' ',
    'S': '.'
}

entity_to_char = {
    '*': 'm',
    'O': 's',
    '#': 'w',
    '+': 'g',
    ' ': 'b',
    '.': 'S'
}


def convert_from_game_id(game_id, max_moves=0):
    match = re.search(r'^#(\d+)x(\d+):([mswgbS]+)$', game_id)
    if match:
        if len(match.groups()) != 3:
            raise Exception("Non valid game id")

        width = int(match.group(1))
        height = int(match.group(2))
        id_string = match.group(3)

        chars = [x for x in id_string]
        chunks = [chars[i:i + width] for i in range(0, len(chars), width)]
        chunks = list(map(lambda c: list(map(lambda x: char_to_entity[x], c)), chunks))
        for chunk in chunks:
            chunk[0:0] = chunk[len(chunk):len(chunk)] = ['#']
        chunks.insert(0, list('#' * (width + 2)))
        chunks.insert(len(chunks), list('#' * (width + 2)))
        chunks = list(map(lambda c: ''.join(c), chunks))
        map_string = '\n'.join(chunks)
        return height + 2, width + 2, max_moves, map_string


def convert_to_game_id(map_string):
    map_string = str(map_string)
    lines = map_string.splitlines()
    lines = list(filter(lambda l: l.startswith('#'), lines))
    del lines[0]
    del lines[-1]
    lines = list(map(lambda l: l[1:len(l) - 1], lines))
    lines = list(map(lambda l: list(map(lambda c: entity_to_char[c], l)), lines))
    lines = list(map(lambda l: ''.join(l), lines))
    id_string = ''.join(lines)

    height = len(lines)
    width = len(lines[0])

    return f'#{width}x{height}:{id_string}'


def main():
    try:
        if len(sys.argv) < 3:
            raise Exception("No arguments provided!")

        game_id = sys.argv[1]
        max_moves = sys.argv[2]

        h, w, m, map_string = convert_from_game_id(game_id, max_moves)
        if len(sys.argv) < 4:
            print(h, w)
            print(m)
            print(map_string)
        else:
            out_file_path = sys.argv[3]
            with open(out_file_path, "w") as text_file:
                print(h, w, file=text_file)
                print(m, file=text_file)
                print(map_string, file=text_file)

    except Exception as e:
        print(e)


if __name__ == '__main__':
    main()
