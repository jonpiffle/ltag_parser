from nltk.corpus.util import LazyCorpusLoader
from nltk.corpus.reader import CategorizedBracketParseCorpusReader
from nltk.tree import Tree, ParentedTree, ImmutableTree

from parser_constants import Constants

import shlex, subprocess, re
from collections import defaultdict

class PosWord(object):
    def __init__(self, wsj_entry, word_num, pos, word):
        self.wsj_entry = wsj_entry
        self.word_num = word_num
        self.pos = pos
        self.word = word

    def propbank_dict(self):
        return self.wsj_entry.propbank_dict()

class WordToTreeMapper(object):
    pos_map = {
        "JJ": "A", # Normal Adjectives: old, red, ect   
        "JJR": "A", # Comparative Adjective: higher, longer, shorter
        "JJS": "A", # Superlative Adjective: highest, most, latest
        "RB": "Ad", # Normal Adverb: quickly; degree word: quite, too, very; posthead modifiers: enough, indeed; negative modifiers: not, never
        "WRB": "Ad", # Wh-adverb: how, why, where
        "CC": "Conj", # Coordinating Conjunction: and, or
        "PRP$": "D", # Possessive pronoun: its, our, 
        "WP$": ["D", "N"] , # Possessive wh-pronoun: whose
        "WP": "N", # Wh pronoun: who, what, whom
        "DT": "D", # Determiner: the, a, every, 
        "POS": "G", # Possessive ending: 's
        "UH": "I", # Interjection: oh, well, indeed
        "NN": "N", # Common Noun: cat, dog
        "NNS": "N", # Plural Noun: cats, dogs
        "NNPS": "N", # Plural Proper Noun
        "NNP": "N", # Proper Noun
        "IN": ["P", "Comp"], # Subordinating Conjunction/Preposition: Subordinating conjunction is IN followed by clause, preposition is IN followed by NP or PP
        "RP": "PL", # Particle: out, in, off, down
        ",": "Punct", # comma
        ".": "Punct", # period
        "$": "Punct", # dollar sign
        "``": "Punct", # weird quote
        "''": "Punct", # quote
        ":": "Punct", # colon
        ")": "Punct", # close paren
        "(": "Punct", # open paren
        "#": "Punct", # pound sign
        "-RRB-": "Punct", # close parent
        "-LRB-": "Punct", # open paren
        "SYM": "N", # Symbol: "a" when listing a,b,c in list
        "VBP": "V", # Verb, non-3rd person singular present: 
        "VBG": "V", # Gerund: running
        "VBD": "V", # Verb, past tense: ran
        "VBZ": "V", # Verb, 3rd person singular present
        "VB": "V", # Verb, base form: to bring, will join, should regulate
        "VBN": "V", # Verb, past particle: was named, have died
        "MD": "V", # Modal verb: will, should, can, may
        "EX": "N", # existential there: there is asbestos
        "FW": "N", # foreign word: Glasnost
        "LS": "Punct", # list item marker
        "PDT": "D" , # Predeterminer: all this, all the assets, half its work
        "WDT": ["N", "Comp"], # Wh determiner: which, that
        "CD": ["D", "N"], # cardinal number: 31, two
        "PRP": "N" , # personal pronoun: I, me, you, he
        "RBR": "Adv", # Adverb, comparative: more, less 
        "RBS": "Adv", # Adverb superlative: most 
        "TO": ["V", "P"] # marker of the word "to": can be a preposition or infinitive marker
    }
    def __init__(self):
        raise NotImplementedError

    def tree_families(self, word_pos):
        ### Get VerbNet classes from PropBank entry ###
        instance = Constants().propbank.get_instance(word_pos.wsj_entry.file_num, word_pos.wsj_entry.sent_num, word_pos.word)
        roleset_id = instance.roleset_id
        role = Constants().propbank.get_role(roleset_id)
        vn_classes = role.vn_classes
        if len(vn_classes) == 0:
            raise ValueError("Propbank has no VNet Mapping")

        ### Get VerbNet frames associated with each VerbNet class ###
        frames = []
        for vn_class in vn_classes:
            frames += Constants().vnet.get_frames_from_class(vn_class)

        ### Get XTAG tree families associated with each frame ###
        tree_families = [Constants().mapper.get_xtag_family(f.primary, f.secondary) for f in frames]
        tree_families = [tf for tf in tree_families if tf is not None]
        tree_families = set(tree_families)

        return tree_families

    def trees(self, word_pos):
        if word_pos.word in word_pos.propbank_dict():
            try:
                return self.trees_from_propbank(word_pos)
            except ValueError as e:
                return self.default_tree_families(word_pos)
        else:
            return self.trees_from_syn(word_pos)

    def default_tree_families(self, word_pos):
        trees = set([("V", "betaVvx"), ("V", "betaVs")])
        for tf in Constants().verbnet_tree_families:
            for t in Constants().grammar.get_trees_from_tree_family(tf):
                # Only want to handle single anchors
                if len(t.anchor_positions()) > 1 or t.anchor_positions()[0].prefix() != 'V':
                    continue

                if word_pos.pos == "VBN" and "G" not in t.tree_name and "D" not in t.tree_name:
                    continue
                if word_pos.pos != "VBN" and ("G" in t.tree_name or "D" in t.tree_name):
                    continue

                # Skip by-phrases if no "by" in sentence
                if not word_pos.wsj_entry.by_in_word_list() and "by" in t.tree_name:
                    continue

                trees.add(("V", t.tree_name))
        return trees

    def trees_from_propbank(self, word_pos,
            single_anchor = True,
            verb_anchor = True,
            arg_num_match = True,
            gerund_match = True,
            by_phrase_match = True,
        ):

        tree_families = self.tree_families(word_pos)
        instance = Constants().propbank.get_instance(word_pos.wsj_entry.file_num, word_pos.wsj_entry.sent_num, word_pos.word)
        trees = set()
        for tf in tree_families:
            for t in Constants().grammar.get_trees_from_tree_family(tf):

                # Only want to handle single anchors
                if single_anchor and len(t.anchor_positions()) > 1:
                    continue

                # Only want to handle verbal anchors
                if verb_anchor and t.anchor_positions()[0].prefix() != 'V':
                    continue

                # Require same number of args
                if arg_num_match and len(t.subst_nodes()) != len(instance.numbered_args()):
                    continue

                # Skip when one is Gerund and other isn't
                if gerund_match and word_pos.pos == "VBN" and "G" not in t.tree_name and "D" not in t.tree_name:
                    continue
                if gerund_match and word_pos.pos != "VBN" and ("G" in t.tree_name or "D" in t.tree_name):
                    continue

                # Skip by-phrases if no "by" in sentence
                if by_phrase_match and not word_pos.wsj_entry.by_in_word_list() and "by" in t.tree_name:
                    continue

                trees.add(("V", t.tree_name))
        return trees

    def trees_from_syn(self, word_pos, single_anchor = True, verbal_anchor = True, skip_tree_fams = True, skip_verb_tree_fams=True):
        word_trees = set()
        for xtag_pos in self.xtag_pos_list(word_pos):
            for _, trees, tree_fams, features in Constants().syntax_dict.get_syn_entries(word_pos.word, xtag_pos):
                word_trees.update(set([(xtag_pos, t.replace("\x02", "alpha").replace("\x03", "beta")) for t in trees]))

                if skip_tree_fams and word_pos.word not in ['old']:
                    continue

                if skip_verb_tree_fams and xtag_pos == 'V':
                    continue

                # Need to get trees from every verb tree
                for tf in tree_fams:
                    for t in Constants().grammar.get_trees_from_tree_family(tf):

                        # Only want to handle single anchors
                        if single_anchor and (len(t.anchor_positions()) > 1 or t.anchor_positions()[0].prefix() != xtag_pos):
                            continue

                        # In simplest case, only allow tree families with verbal anchor
                        if verbal_anchor and xtag_pos != "V":
                            continue

                        word_trees.add((xtag_pos, t.tree_name))

        return word_trees

    def xtag_pos_list(self, word_pos):
        xtag_pos_tup = self.__class__.pos_map[word_pos.pos]
        if isinstance(xtag_pos_tup, str):
            xtag_pos_tup = [xtag_pos_tup]
        return xtag_pos_tup

    def parser_string(self, word_pos):
        pstrings = []
        for xtag_pos, tree in self.trees(word_pos):
            pstring = "%d %s %s %s %s %f" % (word_pos.word_num, word_pos.word, word_pos.pos, xtag_pos, tree, -12.0)
            pstrings.append(pstring)
        return "\n".join(pstrings)

class SimpleTreeMapper(WordToTreeMapper):
    def __init__(self, wsj_entry=None):
        pass

    def trees_from_propbank(self, word_pos):
        return super(SimpleTreeMapper, self).trees_from_propbank(word_pos)

    def trees_from_syn(self, word_pos):
        return super(SimpleTreeMapper, self).trees_from_syn(word_pos)

class ExpandedTreeMapper(WordToTreeMapper):
    def __init__(self, wsj_entry=None):
        pass

    def trees_from_propbank(self, word_pos):
        return super(ExpandedTreeMapper, self).trees_from_propbank(word_pos, arg_num_match=False)

    def trees_from_syn(self, word_pos):
        return super(ExpandedTreeMapper, self).trees_from_syn(word_pos, verbal_anchor=False)

class TreeFamilyExpandedTreeMapper(WordToTreeMapper):
    def __init__(self, wsj_entry=None):
        pass

    def trees_from_propbank(self, word_pos):
        return super(TreeFamilyExpandedTreeMapper, self).trees_from_propbank(word_pos, arg_num_match=False)

    def trees_from_syn(self, word_pos):
        return super(TreeFamilyExpandedTreeMapper, self).trees_from_syn(word_pos, verbal_anchor=False, skip_tree_fams=False)

class VerbTreeFamilyExpandedTreeMapper(WordToTreeMapper):
    def __init__(self, wsj_entry=None):
        pass

    def trees_from_propbank(self, word_pos):
        return super(VerbTreeFamilyExpandedTreeMapper, self).trees_from_propbank(word_pos, arg_num_match=False)

    def trees_from_syn(self, word_pos):
        return super(VerbTreeFamilyExpandedTreeMapper, self).trees_from_syn(word_pos, verbal_anchor=False, skip_tree_fams=False, skip_verb_tree_fams=False)

class OriginalTreeMapper(WordToTreeMapper):
    def __init__(self, wsj_entry):
        self.wsj_entry = wsj_entry
        self.old_mapping_dict = self.parse_old_mapper()

    def trees(self, word_pos):
        old_tree_set = self.old_mapping_dict[word_pos.word_num]
        return old_tree_set

    def parse_old_mapper(self):
        old_mapping_dict = defaultdict(set)
        old_mapper_output = self.run_old_mapper()

        for line in old_mapper_output.split("\n"):
            if "begin sent" in line or line == "end" or line == "":
                continue
            word_num, word, pos, xtag_pos, tree, prob = line.split(" ")
            word_num = int(word_num)
            old_mapping_dict[word_num].add((xtag_pos, tree))

        return old_mapping_dict

    def run_old_mapper(self):
        self.wsj_entry.create_tmp_tagged_file()

        p1_cmd = shlex.split("echo \"%s\"" % self.wsj_entry.sentence())
        p1 = subprocess.Popen(p1_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        # Get the syntax entries
        p2_cmd = shlex.split("%s/bin/syn_get.bin %s/data/english/english.grammar %s/lib/xtag.prefs" % (Constants().parser_dir, Constants().parser_dir, Constants().parser_dir))
        p2 = subprocess.Popen(p2_cmd, stdin=p1.stdout, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        # Filter the syntax entries by part-of-speech
        p3_cmd = shlex.split("%s/bin/tagger_filter %s" % (Constants().parser_dir, self.wsj_entry.tmp_tagged_filename()))
        p3 = subprocess.Popen(p3_cmd, stdin=p2.stdout, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)

        parser_output, err = p3.communicate()
        return parser_output

class UnionTreeMapper(WordToTreeMapper):
    def __init__(self, wsj_entry):
        self.expanded_mapper = VerbTreeFamilyExpandedTreeMapper()
        self.original_mapper = OriginalTreeMapper(wsj_entry)

    def trees(self, word_pos):
        original_tree_set = self.original_mapper.trees(word_pos)
        expanded_tree_set = self.expanded_mapper.trees(word_pos)
        return original_tree_set | expanded_tree_set

'''
words, _ = zip(*sentence)
print("begin sent=\"%s\"" % (" ".join(words)))
for i, (word, pos) in enumerate(sentence):
    word_trees, word_tree_fams = set(), set()
    for xtag_pos in xtag_pos_tup:
        for word_pos, trees, tree_fams, features in syntax_dict.get_syn_entries(word, xtag_pos):
            word_trees.update([(xtag_pos, t) for t in trees])
            word_tree_fams.update([(xtag_pos, tf) for tf in tree_fams])

    for xtag_pos, tree_name in word_trees:
        print("%d %s %s %s %s %f" % (i, word, pos, xtag_pos, tree_name, -12.0))

    for xtag_pos, tf in word_tree_fams:
        for t in grammar.get_trees_from_tree_family(tf):
            if len(t.anchor_positions()) > 1 or t.anchor_positions()[0].prefix() != xtag_pos:
                continue
            print("%d %s %s %s %s %f" % (i, word, pos, xtag_pos, t.tree_name, -12.0))

print("end")
'''
