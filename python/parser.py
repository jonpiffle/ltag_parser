import preprocess
from word_to_trees import PosWord, SimpleTreeMapper, ExpandedTreeMapper, TreeFamilyExpandedTreeMapper, VerbTreeFamilyExpandedTreeMapper, OriginalTreeMapper, UnionTreeMapper
from parser_constants import Constants

import shlex, subprocess, re, os, json
from multiprocessing import Pool

class WSJEntry(object):
    def __init__(self, file_id, sent_num, parse_tree):
        self.file_id = str(file_id)
        self.file_num = file_id.split("/")[-1].split(".")[0].replace("wsj_", "")
        self.sent_num = str(sent_num)
        self.parse_tree = parse_tree

    def tagged(self):
        tag = preprocess.parse_to_tagged(self.parse_tree)
        return preprocess.remove_none_from_tagged(tag)

    def tagged_str(self):
        return " ".join([word + '_' + pos for pos, word in self.tagged()])

    def word_list(self):
        return preprocess.tagged_to_sent(self.tagged()) 

    def sentence(self):
        return preprocess.sent_to_str(self.word_list())

    def preprocess(self):
        self.parse_tree = preprocess.merge_tree_nnps(self.parse_tree)
        self.parse_tree = preprocess.remove_punc_from_tree(self.parse_tree)
        self.parse_tree = preprocess.lowercase_tree_first_word(self.parse_tree)
        return self

    def by_in_word_list(self):
        return "by" in self.word_list()

    def pos_words(self):
        pwords = []
        for i, (pos, word) in enumerate(self.tagged()):
            pos_word = PosWord(self, i, pos, word)
            pwords.append(pos_word)
        return pwords

    def propbank_dict(self):
        return Constants().propbank.instance_dict[(self.file_num, self.sent_num)]

    def parser_string(self, tree_mapper):
        pstrings = ["begin sent=\"%s\"" % self.sentence()]
        pstrings += [tree_mapper.parser_string(pos_word) for pos_word in self.pos_words()]
        pstrings += ["end"]
        return "".join([pstring + "\n" for pstring in pstrings]) # Trailing newline is mandatory

    def tmp_tagged_filename(self):
        return '%s/test/tagged/%s_%s.tagged' % (Constants().parser_dir, self.file_num, self.sent_num)

    def parser_output_filename(self):
        return '%s/parser_output/%s_%s.txt' % (Constants().parser_dir, self.file_num, self.sent_num)

    def parser_input_filename(self):
        return '%s/parser_output/%s_%s.input' % (Constants().parser_dir, self.file_num, self.sent_num)

    def parser_status_filename(self):
        return '%s/parser_output/%s_%s.status' % (Constants().parser_dir, self.file_num, self.sent_num)

    def write_status(self, status):
        with open(self.parser_status_filename(), 'w') as f:
            f.write(json.dumps(status))

    def create_tmp_tagged_file(self):
        filename = self.tmp_tagged_filename()
        with open(filename, 'w') as f:
            f.write(self.tagged_str() + "\n")
        return filename

    def parse(self, parser_input, timeout, tree_mapper):
        with open(self.parser_input_filename(), 'w') as f:
            f.write(parser_input)

        p1_cmd = shlex.split("cat %s" % self.parser_input_filename())
        p1 = subprocess.Popen(p1_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        p2_cmd = shlex.split("%s/bin/nbest_parser.bin %s/data/english/english.grammar %s/lib/xtag.prefs" % (Constants().parser_dir, Constants().parser_dir, Constants().parser_dir))
        p2 = subprocess.Popen(p2_cmd, stdin=p1.stdout, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)

        try:
            parser_output, err = p2.communicate(timeout=timeout)
        except subprocess.TimeoutExpired:
            self.write_status({'action': 'FAILED', 'reason': "TIMEOUT", "time": str(timeout)})
            p2.kill()
            return True

        with open(self.parser_output_filename(), 'w') as f:
            f.write(parser_output)

        derivation_count = self.count_derivations()
        if derivation_count > 0:
            self.write_status({'action': 'SUCCESS', 'mapper': str(tree_mapper)})

        return derivation_count > 0 # Determines if we need to retry the parse or not

    def count_derivations(self):
        p1_cmd = shlex.split("cat %s" % self.parser_output_filename())
        p1 = subprocess.Popen(p1_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        p2_cmd = "%s/bin/count_derivations" % (Constants().parser_dir)
        p2 = subprocess.Popen(p2_cmd, stdin=p1.stdout, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
        output, err = p2.communicate()

        count_str = re.search('count=(\d+)', output)
        if count_str is None:
            return 0

        num_derivations = int(count_str.group(1))
        return num_derivations

    def __str__(self):
        return "<WSJEntry: %s %s '%s'>" % (self.file_id, self.sent_num, self.sentence())

def parse(file_id, sent_num, parse_tree, timeout):
    entry = WSJEntry(file_id, sent_num, parse_tree)
    entry = entry.preprocess()

    tree_mappers = [SimpleTreeMapper, ExpandedTreeMapper, TreeFamilyExpandedTreeMapper, VerbTreeFamilyExpandedTreeMapper, OriginalTreeMapper, UnionTreeMapper]
    parser_inputs = [entry.parser_string(tree_mapper(entry)) for tree_mapper in tree_mappers]
    print([len(i) for i in parser_inputs])

    for tree_mapper, parser_input in sorted(zip(tree_mappers, parser_inputs), key=lambda x: len(x[1])):
        print(entry, tree_mapper)

        # Some parser feedback
        entry.write_status({'action': 'PARSING', 'mapper': str(tree_mapper)})

        parser_result = entry.parse(parser_input, timeout, tree_mapper)

        if parser_result:
            return

    entry.write_status({'action': 'FAILED', 'reason': "NOPARSEFOUND"})

def main(processes=1):
    ptb = Constants().ptb
    timeout = 60
    fileids = list(ptb.fileids())[0:10]

    params = []
    for fileid in fileids:
        for sent_num, parse_tree in enumerate(ptb.parsed_sents(fileid)):
            params.append((fileid, sent_num, parse_tree, timeout))

    if processes > 1:
        p = Pool(processes)
        p.starmap(parse, sorted(params, key=lambda x: (x[0], x[1])))
    else:
        for param in params:
            parse(*param)

if __name__ == "__main__":
    main(processes=3)
