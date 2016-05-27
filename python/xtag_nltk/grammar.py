import nltk, os, pickle
from xtag_nltk.load import get_catalog, get_file_list, get_start_feature,\
    analyze_morph, analyze_syntax, analyze_template, make_pos_mapping,\
    parse_from_files
from xtag_nltk.feature import remove_or_tag

class Grammar(object):
    def __init__(self, language):
        self.language = language
        self.catalog = Catalog(language) 
        self.trees = self.load_trees()
        self.syntax = Syntax.from_catalog(self.catalog)
        self.morphology = Morphology.from_catalog(self.catalog, self.syntax)
        self.template = Template.from_catalog(self.catalog)
        self.pos_mapping = self.load_pos_mapping()

    @classmethod
    def load(cls, language):
        pickle_filename = '%s_grammar.pickle' % language
        if os.path.exists(pickle_filename):
            return pickle.load(open(pickle_filename, 'rb'))
        else:
            grammar = Grammar(language)
            pickle.dump(grammar, open(pickle_filename, 'wb'))
            return grammar

    def load_trees(self):
        sfs = get_start_feature(self.catalog.cata)
        sfs = remove_or_tag(sfs)
        t = parse_from_files(self.catalog.cata, 'tree-files', self.language)
        t += parse_from_files(self.catalog.cata, 'family-files', self.language)
        t.set_start_fs(sfs)
        return t

    def load_pos_mapping(self):
        mapping = self.catalog.load_resource('mapping')
        return make_pos_mapping(mapping)

    def word_to_morphologies(self, word, pos=None):
        """
        Returns the morphological entries for a given word

        A morphological entry is a tuple of (lemma, pos, [feature list])

        To find the list of entries, first check if there is an entry in the
        morphology dictionary for the word. If not, check if there is an entry
        in the syntax_dict for the word. If not, use default morphology dict
        entry.

        If a pos is supplied, filter morphology entries by pos.
        """

        if self.morphology.exists(word):
            morph = self.morphology.get_morph_entries(word)
        elif self.syntax.exists(word):
            morph = []
            for entry in self.syntax.get_syn_entries(word):
                for word, pos in entry[0]:
                    morph.append((word, pos, []))
        else:
            morph = self.morphology.get_morph_entries(word)

        if pos is not None:
            morph = [m for m in morph if m[1] == pos]
            if len(morph) == 0:
                morph = [self.morphology.get_default_morph_entry(pos)]

        return morph

    def morphologies_to_features(self, morph_entries, word, pos):
        """
        Returns a list of features given a list of morph entries

        Takes a list of morph entries in the form of (word, pos, [feature list])
        Returns a list of features of the form:
            ([word_pos list], [tree names], [tree families], [syntax features],
             [morph features])
        
        """
        feature_list = []
        for morph_entry in morph_entries: 
            syn_entries = self.syntax.get_syn_entries(word, pos)
            for entry_pos_list, tree_list, family_list, syn_feature_list in syn_entries:
                morph_feature, syn_feature = [], []

                for feature in syn_feature_list: 
                    syn_feature.append(self.template.get_syntax_template(feature[1:]))

                for feature in morph_entry[2]:
                    morph_feature.append(self.template.get_morph_template(feature))

                feature_list.append((entry_pos_list, tree_list, family_list, syn_feature, morph_feature))
        return feature_list

    def get_lexicalized_trees(self, word, pos=None):
        """
        Takes a word and an optional pos and returns all trees lexicalized with 
        the given word.
        """

        morph_entries = self.word_to_morphologies(word, pos)
        feature_list = self.morphologies_to_features(morph_entries, word, pos)

        lex_trees = []
        for entry_pos_list, tree_list, family_list, syn_feature, morph_feature in feature_list:
            tree_names = set(tree_list)
            for family in family_list:
                tree_names.update(set(self.trees.family_to_tree_names(family)))

            for tree_name in tree_names:
                tree = self.trees.find_tree(tree_name).copy(deep=True)
                tree.init_lex(entry_pos_list, remove_or_tag(syn_feature), remove_or_tag(morph_feature))
                tree.lexicalize()
                lex_trees.append(tree)

        return lex_trees

    def get_lexicalized_tree(self, word, tree_name):
        """Takes a word and a tree and returns tree lexicalized with word """

        family_name = self.trees.tree_name_to_family(tree_name)
        tree = self.trees.find_tree(tree_name).copy(deep=True)
        pos = tree.get_head_name()

        # Only allow morphologies with a single lexicalization for now
        morph_entries = self.word_to_morphologies(word, pos)
        morph_entry = [m for m in morph_entries if m[1] == pos]

        # Only allow features with a single lexicalization for now
        feature_list = self.morphologies_to_features(morph_entry, word, pos)
        feature_list = [f for f in feature_list if len(f[0]) == 1]

        # Prioritize features that explicitly list the tree
        # Don't have reason to prefer a feature dict, so just take first
        matched_feature_list = [f for f in feature_list if tree_name in f[1] or family_name in f[2]]
        if len(matched_feature_list) == 0:
            feature = feature_list[0]
        else:
            feature = matched_feature_list[0]

        entry_pos_list, tree_list, family_list, syn_feature, morph_feature = feature
        tree.init_lex(entry_pos_list, syn_feature, morph_feature)
        tree.lexicalize()
        return tree

class Catalog(object):
    """
    Holds information for where to find each component of xtag resources
    """
    
    def __init__(self, language):
        self.language = language
        self.xtag_dir = 'xtag_grammar'

        cata_dir = 'xtag_grammar/'+language+'/'+language+'.gram'
        with open(nltk.data.find(cata_dir), 'r') as f:
            cata_str = f.read()
        self.cata = get_catalog(cata_str)

    def load_resource(self, resource_name):
        return open(nltk.data.find(self.get_resource_path(resource_name)), 'r').read()

    def get_resource_path(self, resource_name):
        if resource_name == 'mapping':
            return os.sep.join([self.xtag_dir, self.language, 'syntax_morph.mapping'])
        else:
            return self.filelist_to_path(get_file_list(self.cata, resource_name))

    def filelist_to_path(self, filelist):
        return os.sep.join([self.xtag_dir, self.language, filelist[1], filelist[0][0]])

class Morphology(object):
    """
    Provides a mapping from words to (lemma, POS, [features])
    """

    def __init__(self, morph_dict):
        self.morph_dict = morph_dict

    def get_morph_entries(self, word):
        if word in self.morph_dict:
            return self.morph_dict[word]
        else:
            return self.morph_dict['%s']

    def get_default_morph_entries(self):
        entries = self.morph_dict['%s'] 
        return entries

    def get_default_morph_entry(self, pos):
        entries = self.get_default_morph_entries()
        entry = [e for e in entries if e[1] == pos][0]
        return entry

    def exists(self, word):
        return word in self.morph_dict

    @classmethod
    def from_catalog(cls, catalog, syntax):
        morph = catalog.load_resource('morphology-files')
        morph_dict = analyze_morph(morph)
        default_morph_dict = [('%s', s[0][0][1], []) for s in syntax.get_syn_entries('%s')]
        morph_dict['%s'] = default_morph_dict
        return Morphology(morph_dict)

class Syntax(object):
    """
    Provides a mapping from words to lemma, POS, trees, and features

    Examples
    -------
    bullying => 
        [([('bullying', 'V')], ['\x02nx1V', '\x03Ncnx1V', '\x02Inx0Vnx1', 
         '\x03N1nx1V', '\x02AV', '\x03N0nx0Vnx1'], ['Tnx0Vnx1'], ['#V.m_PROG'])]
    """

    def __init__(self, word_to_tree):
        self.word_to_tree = word_to_tree

    def exists(self, word):
        return word in self.word_to_tree

    def get_syn_entries(self, word, pos=None):
        #word = word.lower()
        if word.lower() in self.word_to_tree:
            syn_entries = self.word_to_tree[word.lower()]
        else:
            syn_entries = self.word_to_tree['%s']

        # Filter based on pos
        if pos is not None:
            new_syn_entries = []
            for syn_entry in syn_entries:
                if (word, pos) in syn_entry[0]:
                    new_syn_entries.append(syn_entry)

            # If no matching entries, use default for POS
            if len(new_syn_entries) == 0:
                syn_entries = self.word_to_tree['%s']
                for syn_entry in syn_entries:
                    entry_pos_list, tree_list, family_list, syn_feature_list = syn_entry
                    if ('%s', pos) in entry_pos_list:
                        # Need to copy entry_pos_list to avoid changing syn entry
                        entry_pos_list = list(entry_pos_list)
                        entry_pos_list.remove(('%s', pos))
                        entry_pos_list.append((word, pos))
                        new_syn_entries.append((entry_pos_list, tree_list, family_list, syn_feature_list))

            syn_entries = new_syn_entries

        return syn_entries

    @classmethod
    def from_catalog(cls, catalog):
        syn = catalog.load_resource('lexicon-files')
        default = catalog.load_resource('syntax-default')
        s = syn + "\n" + default
        word_to_tree = analyze_syntax(s)
        return Syntax(word_to_tree)


class Template(object):
    """
    There are two kinds of templates, one of them is started by a @ mark, which is used
    in the morph file, and another type is started with a # mark, which is used
    in the syntax file. When we are constructing trees with a particular anchor,
    the code will search the templates of features and combine them with the xtag
    tree.

    A template maps a feature value to a featstruct.FeatDict

    Example:

    3pl => [agr=[3rdsing=[__or_-='-'], num=[__or_plur='plur'], pers=[__or_3='3']]]
    """

    def __init__(self, morph_dict, syntax_dict):
        self.morph_dict = morph_dict
        self.syntax_dict = syntax_dict

    def get_morph_template(self, feature):
        return self.morph_dict[feature]

    def get_syntax_template(self, feature):
        return self.syntax_dict[feature]

    @classmethod
    def from_catalog(cls, catalog):
        template = catalog.load_resource('templates-files')
        morph_dict, syntax_dict = analyze_template(template)
        return Template(morph_dict, syntax_dict)


def test_relative_clauses():
    from xtag.xtag_nltk.xparse import perform_substitution, perform_adjunction, perform_all_substitutions, perform_all_adjunctions
    from xtag.xtag_nltk.feature import remove_or_tag
    g = Grammar('english')

    dog_tree = g.get_lexicalized_tree('dog', '\x02NXN')
    chase_tree = g.get_lexicalized_tree('chase', '\x03Nc0nx0Vnx1')
    t = perform_all_adjunctions(dog_tree, chase_tree, feature_enabled=True)[0]

    that_tree = g.get_lexicalized_tree('that', '\x03COMPs')
    adj_node_success = t.search('S_2')
    adj_node_fail = t.search('S_p')

    t2 = perform_adjunction(t, that_tree, adj_node_success, feature_enabled=True)
    t3 = perform_adjunction(t, that_tree, adj_node_fail, feature_enabled=True)

    print(remove_or_tag(t2.get_all_fs()))
    t2.draw()


if __name__ == '__main__':
    #g = Grammar('english')
    #lex_tree = g.get_lexicalized_tree('chase', "\x02Gnx0Px1")
    #print(lex_tree.tree_name)
    #lex_tree.draw()
    #import code; code.interact(local=locals())
    test_relative_clauses()

