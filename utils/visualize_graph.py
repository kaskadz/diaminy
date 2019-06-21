import sys
from graphviz import Source


def main():
    try:
        if len(sys.argv) < 2:
            raise Exception("No arguments provided!")

        graph_file_path = sys.argv[1]

        with open(graph_file_path, 'r') as dot_file:
            dot_content = dot_file.read()

        s = Source(dot_content, format='svg')
        s.view()
    except Exception as e:
        print(e)


if __name__ == '__main__':
    main()
