import json, glob, os
import shlex, subprocess
from collections import defaultdict
from nltk import Tree

def count_successful_parses(parser_output_dir='revised_parser_output'):
    result_counts = defaultdict(int)

    status_files = glob.glob(parser_output_dir + '/*.status')
    for status_file in status_files:
        status = json.load(open(status_file, 'r'))
        result_counts[status['action']] += 1 

    print(result_counts)
    print('status_files', len(status_files))

def parser_output_to_parse_deriv_trees(output):
    lines = output.strip().split("\n")
    deriv_tree_lines = lines[::2]
    parse_tree_lines = lines[1::2]

    parse_trees = [Tree.fromstring(line.replace('\x06', 'epsilon_')) for line in parse_tree_lines if line != '']
    deriv_trees = [Tree.fromstring(line) for line in deriv_tree_lines if line != '']
    return parse_trees, deriv_trees

def sample_tree_from_parser_output(parser_output_filename):
    p1_cmd = shlex.split("cat %s" % parser_output_filename)
    p1 = subprocess.Popen(p1_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    p2_cmd = "../bin/truncate_graph"
    p2 = subprocess.Popen(p2_cmd, stdin=p1.stdout, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    p3_cmd = shlex.split("../bin/print_deriv -b")
    p3 = subprocess.Popen(p3_cmd, stdin=p2.stdout, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    output, err = p3.communicate()

    parse_trees, deriv_trees = parser_output_to_parse_deriv_trees(output)

    return parse_trees[0], deriv_trees[0]

def generate_sample_parses(parser_output_dir='revised_parser_output', parse_tree_output_dir='revised_parse_trees'):
    status_files = glob.glob(parser_output_dir + '/*.status')
    for i, status_file in enumerate(status_files):
        print('processing %d of %d' % (i+1, len(status_files)))
        
        status_dict = json.load(open(status_file, 'r'))
        status = status_dict["action"]

        if status != 'SUCCESS':
            continue

        parser_output_filename = status_file.replace('status', 'txt')
        tree_filename = parser_output_filename.replace(parser_output_dir, parse_tree_output_dir)

        if os.path.exists(tree_filename):
            continue

        try:
            parse, deriv = sample_tree_from_parser_output(parser_output_filename)
        except:
            continue

        tree_dict = {"parse": str(parse), "deriv": str(deriv)}

        with open(tree_filename, 'w') as f:
            f.write(json.dumps(tree_dict))

if __name__ == '__main__':
    generate_sample_parses()
    #count_successful_parses()
