# -*- coding: utf-8 -*-
# Natural Language Toolkit: Tree-Adjoining Grammar
#
# Copyright (C) 2001-2013 NLTK Project
# Author: WANG Ziqi, Haotian Zhang <{zwa47,haotianz}@sfu.ca>
#
# URL: <http://www.nltk.org/>
# For license information, see LICENSE.TXT
#

import os, nltk
from nltk.featstruct import *

from xtag.xtag_nltk.feature import *
from xtag.xtag_nltk.tagtree import TAGTree, TAGTreeSet

import xtag.xtag_nltk.LL1 as LL1

###########################################
# LL Parser for catalog file ##############
###########################################

# Do not call these functions from the outside! Only be used by the parser

def get_next_token(s,index,length=None):
    """
    To get the next token in english.gram. It is part of the LL parser.
    :param s: The source string
    :type s: str
    :param index: Current reading position.
    :type index: integer
    :param length: The length of s. Can be None. Just make it faster.
    :type length: integer

    :return: The token and current reading position
    :rtype: tuple(str,integer)
    """
    # This function is used by the LL parser to extract out tokens from the input
    # stream, which is not a general one, but is designed and simplified just
    # for use in the XTAG case. DO NOT USE IT AS A PUBLIC FUNCTION.
    # The return value is a tuple, the first element is the token itself, and
    # if we have reached the end of the string, this value should be a None
    # instead of some certain string. The second element of the tuple is the
    # next character index, after extracting the token, and if the first
    # element is None, the second element is the index that we passed in.
    # i.e. no change at all, this is designed to prevent repeated invalid call.
    #################
    # state = 0: outside anything
    # state = 1: inside ""
    # state = 2: inside identifier
    # state = 3: inside comment (not for sure)
    state = 0
    start = 0
    end = 0
    # length is the length of s, just for speed up if the length can be
    # calculated outside of this function and doesn't change
    if length == None:
        l = len(s)
    else:
        l = length

    while True:
        # If we have already reached the end of the string, index >= l (length)
        if state == 0 and index >= l:
            return (None,index)
        # Jump all space characters
        elif state == 0 and s[index].isspace():
            index += 1
        # Single '(' or ')' or ':' can itself form a token without any indicators
        elif state == 0 and (s[index] == '(' or s[index] == ')' or s[index] == ':'):
            return (s[index],index + 1)
        # If we have seen a '"' then enter a state that will accept anything
        # until another '"', this is used to extract out strings.
        elif state == 0 and s[index] == '"':
            start = index
            state = 1
            index += 1
        # See above.
        elif state == 1 and s[index] == '"':
            end = index
            state = 0
            return (s[start:end + 1],index + 1)
        # all numbers and characters as well as "'" without any space in the
        # middle will be treated in a whole.
        elif state == 0 and (s[index].isalnum() or s[index] == "'"):
            state = 2
            start = index
            index += 1
        # Any space, ')', '(' will terminate an identifier in state 2.
        elif state == 2 and (s[index].isspace() or s[index] == ')' or s[index] == '('):
            end = index - 1
            return (s[start:end + 1],index)
        # Skip all comments until the end of line (indicated by a '\n')
        elif state == 0 and (s[index] == ';' or s[index] == '#'):
            state = 3
            index += 1
        # See above
        elif state == 3 and s[index] == '\n':
            state = 0
            index += 1
        # NOTICE: this is not a exception, many branches above will rely on this
        # else statement, for it is the default processor for many other generic cases
        # not listed above.
        else:
            index += 1

catalog_name = {} # Global names defined by a "setf" statement

def add_new_name(s,index):
    """
    Extract a token from the string as a name, and add the name into the table.
    If there is a name clash then just raise an error
    
    :param s: The source string
    :type s: str
    :param index: Current reading position
    :type index: integer
    :return: The token and current reading position.
    :rtype: tuple(str,integer)
    """
    # This function will add a global name, i.e. those defined after 'setf'
    # into a global dictionary, called catalog_name, and will be retrieved by
    # generate_string later.
    global catalog_name
    token_1 = get_next_token(s,index)
    index = token_1[1]
    token_2 = get_next_token(s,index)
    index = token_2[1]
    if token_1[0] in catalog_name:
        print("WARNING: Name collide - Normal if you have called install() before.")
    if token_2[0][0] != '"' or token_2[0][-1] != '"':
        raise TypeError('add_new_name: Value must be wrapped with "')
    else:
        catalog_name[token_1[0]] = token_2[0][1:-1]
    return (token_2[0][1:-1],index)

def generate_string(s,index):
    """
    Only "concatenate 'string" method is supported.

    :param s: Same as in get_next_token()
    :type s: Same as in get_next_token()
    :param index: Same as in get_next_token()
    :type index: Same as in get_next_token()
    :return: Evaluated string
    :rtype: Same as in get_next_token()
    """
    # This function will generate a string using the defined identifier name
    # i.e. catalog_name. This will be called if a "concatenate 'string" is encountered.
    global catalog_name
    token = get_next_token(s,index)
    index = token[1]
    string_value = ""
    if token[0] == "'string":
        while True:
            token = get_next_token(s,index)
            if token[0][0] == '"' and token[0][-1] == '"':
                index = token[1]
                string_value += token[0][1:-1]
            elif token[0] in catalog_name:
                index = token[1]
                string_value += catalog_name[token[0]]
            else:
                return (string_value,index)
    else:
        raise TypeError("generate_string: Don't support concatenate except \"'string\"")

def evaluate_expression(s,index):
    """
    Evaluate "setf" and "concatenate" expressions and get a string
    :param s: Same as in get_next_token()
    :type s: Same as in get_next_token()
    :param index: Same as in get_next_token()
    :type index: Same as in get_next_token()
    :return: Evaluated string
    :rtype: Same as in get_next_token()
    """
    # This function will evaluate those "expressions" defined by 'setf' or
    # 'concatenate', and dispatch them to the corresponding functions
    # No other expression type are supported for now
    token = get_next_token(s,index)
    if token[0] != '(':
        raise TypeError("evaluate_expression: '(' expected.")
    index = token[1]
    token = get_next_token(s,index)
    index = token[1]
    if token[0] == 'setf':
        ret = add_new_name(s,index)
        index = ret[1]
    elif token[0] == 'concatenate':
        ret = generate_string(s,index)
        index = ret[1]
    else:
        raise TypeError("evaluate_expression: Unknown method")
    token = get_next_token(s,index)
    index = token[1]
    if token[0] != ')':
        raise TypeError("evaluate_expression: ')' expected")
    else:
        return (ret[0],index)

def parse_expression(s,index):
    """
    Get a expression strated with "setf" and "concatenate"

    :param s: Same as in get_next_token()
    :type s: Same as in get_next_token()
    :param index: Same as in get_next_token()
    :type index: Same as in get_next_token()

    :return: A node in the parse tree together with the current reading position.
    :rtype: tuple(tuple(str,list(str)),integer)
    """
    # This function is used to parse an expression. i.e. those strings identified
    # by '"' at the start and the end, and also other types, such as 'setf'
    # and 'concatenate'.
    values = []
    while True:
        token = get_next_token(s,index)
        
        if token[0][0] == '"' and token[0][-1] == '"':
            values.append(token[0][1:-1])
            index = token[1]
        elif token[0][0] == '(':
            token_1 = get_next_token(s,token[1])
            if token_1[0] == ':': # A nested option list (identified with '(:')
                return [("expr",values),index]
            else:
                exp_value = evaluate_expression(s,index) # index is still on '('
                values.append(exp_value[0]) # parse "(something)"
                index = exp_value[1]
        else:
            return [("expr",values),index]

def parse_option(s,index):
    """
    An option started with ":"
    :param s: Same as in get_next_token()
    :type s: Same as in get_next_token()
    :param index: Same as in get_next_token()
    :type index: Same as in get_next_token()

    :return: A parsing tree node
    :rtype: tuple
    """
    # This function will parse an option and return the tree, each node of which
    # is actually a option entry of that option. An option is defined as being strated
    # by a ':' symbol, the format of which is :[option name] [option value]
    token = get_next_token(s,index)
    if token[0] != ':':
        return (None,index)
    else:
        index = token[1]
        t = get_next_token(s,index)
        option_name = t[0]
        index = t[1]
        exp = parse_expression(s,index)
        index = exp[1]
    return [("optn",option_name,exp[0]),index]

def parse_option_set(s,index):
    """
    Parse an option set starting with (: and end with )
    :param s: Same as in get_next_token()
    :type s: Same as in get_next_token()
    :param index: Same as in get_next_token()
    :type index: Same as in get_next_token()

    :return: A parsing tree node
    :rtype: list
    """
    # This function is used to parse an option set, which starts with a "(:"
    # and ends with a ")". There can be option entries or even another option
    # in an option, so that it is defined as naturally nested. Also we must
    # distinguish between an sub-option and option entry inside an option.
    token = get_next_token(s,index)
    if token[0] != '(':
        # push back the token just fetched by returning index
        return (None,index)

    options = []
    index = token[1]
    while True:
        opt = parse_option(s,index)
        index = opt[1]
        if opt[0] == None:
            opt = parse_option_set(s,index)
            index = opt[1]
            if opt[0] == None:
                break
            else:
                options.append(opt[0])
        else:
            options.append(opt[0])

    token = get_next_token(s,index)
    index = token[1]
    if token[0] != ')':
        raise TypeError("parse_option_set: ')' expected.")

    return [("opts",options),index]

def parse_language(s,index):
    """
    Parse a language in the english.gram file
    :param s: Same as in get_next_token()
    :type s: Same as in get_next_token()
    :param index: Same as in get_next_token()
    :type index: Same as in get_next_token()

    :return: A parsing tree node
    :rtype: list
    """
    # This function is called to parse a language in the .grammar file, which
    # starts with "(defgrammar [language name]" and ends with ")". There can
    # be only options in the language. And the parse supports more than one
    # language defined in one file, which is represented as a node under
    # the "top" node. 
    token = get_next_token(s,index)
    if token[0] != '(':
        return (None,index)

    token = get_next_token(s,token[1])
    if token[0] != 'defgrammar':
        raise TypeError("parse_language: 'defgrammar' expected.")

    token = get_next_token(s,token[1])
    language_name = token[0]

    option_set = []
    index = token[1]
    while True:
        opt = parse_option_set(s,index)
        index = opt[1]
        if opt[0] == None:
            break
        else:
            option_set.append(opt[0])

    token = get_next_token(s,index)
    if token[0] != ')':
        raise TypeError("parse_language: ')' expected")
    else:
        return [("lang",language_name,option_set),token[1]]
    
def parse_catalog(s,index):
    """
    Prase the whole english.gram file
    :param s: Same as in get_next_token()
    :type s: Same as in get_next_token()
    :param index: Same as in get_next_token()
    :type index: Same as in get_next_token()

    :return: A parsing tree root
    :rtype: tuple
    """
    # This function parses the catalog represented in a string. The return value
    # is a node of the tree whose content is just the options and other
    # elements of the catalog string. The formats are presented below:
    # node[0] == top: A top node
    #     node[1] ~ end = languages
    # node[0] == lang: A language node
    #     node[1] = language name
    #     node[2] ~ end = option sets
    # node[0] == opts: An option set
    #     node[1] ~ end = option
    # node[0] == optn: An option entry
    #     node[1] = option entry name
    #     node[2] ~ end = option entry values
    # node[0] == expr: An expression node
    #     node[1] ~ end = expression values
    #
    # DON'T DIRECTLY CALL THIS
    #
    top = []
    while True:
        lang = parse_language(s,index)
        if lang[0] == None:
            break
        else:
            top.append(lang[0])
            index = lang[1]
            
    return ("top",top)

def get_catalog(content):
    """
    Given a string of english.gram it will return the parsing tree
    
    :param content: The content of english.gram file
    :type content: str
    :return: The root of the parsing tree
    :rtype: The valued returned by the parsing algorithm
    """
    s = content
    cata = parse_catalog(s,0)
    return cata

# These three is used to extract a certain sub-tree from the parse tree
# and is used during recursion, so don't modify or read the content of
# these variables, the values are not defined.
file_list = None
suffix = None
path = None
opt_value = None
################

def get_file_tree(node,type_string):
    """
    Given the file type this function will return a subtree which is of the
    given file type.
    
    :param node: The tree root or a subtree.
    :type node: The tree node
    :param type_string: The file type you want
    :type type_string: str
    """
    # DON'T DIRECTLY CALL THIS
    # Given the root node of the parsing tree, and the file type that you want
    # to extract, this function will return the tree containing the list
    # of the files of that type.
    global file_list,suffix,path
    ret = None
    if node[0] == "top":
        for lang in node[1]:
            ret = get_file_tree(lang,type_string)  
    elif node[0] == 'lang':
        for opts in node[2]:
            ret = get_file_tree(opts,type_string)
    elif node[0] == "opts":
        for optn in node[1]:
            if optn[1] == type_string:
                file_list = node[1][0][2][1]
                type_suffix = node 
                suffix = node[1][1][1][1][2][1][0]
                path = node[1][1][1][0][2][1][0]
                return None
            else:
                ret = get_file_tree(optn,type_string)
    elif node[0] == 'optn':
        ret = get_file_tree(node[2],type_string)
    elif node[0] == 'expr':
        pass
    else:
        raise TypeError("Unknown keyword in the catalog: " + node[0])
    return ret

def get_file_list(node,type_string):
    """
    This function is called to get the file list of a certain type.
    
    :param node: The parsing tree node
    :type node: Tree node returned by the parsing algorithm
    :param type_string: The type of files you want to get
    :type type_string: str
    """
    # Given the root node of the parse tree, and the file type
    # you want to get, this function will return a list containing
    # the file names as well as the absolute path of the files.
    # PLEASE CALL THIS ONE
    global file_list,suffix,path
    get_file_tree(node,type_string)
    file_list_full = []
    if suffix == 'db':
        suffix_str = 'flat'
    else:
        suffix_str = suffix
    for i in file_list:
        file_list_full.append(i + '.' + suffix_str)
    return (file_list_full,path[:]) # We must copy the path string


def get_option_value(node,opt_name):
    """
    get_option_value(node,opt_name) -> string

    Given the catalog tree and the option name, this function will output the
    option value, which is a string.

    The option can be any options existing in the parse tree, but if multiple
    same option value exists, this function will return the one that emerges
    first in a pre-order traverse.

    If this function fails, i.e. no option with name opt_name is found, then
    it will return a None. So to detect None return value is necessary.

    Example:
    get_option_value(cata,'start-feature')
    Return:
    <mode> = ind/imp <comp> = nil <wh> = <invlink>  <punct term> = per/qmark/excl <punct struct> = nil
    """
    if node[0] == "top":
        for lang in node[1]:
            ret = get_option_value(lang,opt_name)
            if ret != None:
                return ret
    elif node[0] == 'lang':
        for opts in node[2]:
            ret = get_option_value(opts,opt_name)
            if ret != None:
                return ret
    elif node[0] == "opts":
        for optn in node[1]:
            ret = get_option_value(optn,opt_name)
            if ret != None:
                return ret
    elif node[0] == 'optn':
        if node[1] == opt_name:
            return node[2][1][0]
        else:
            ret = get_option_value(node[2],opt_name)
            if ret != None:
                return ret
    elif node[0] == 'expr':
        pass
    return None

def get_start_feature(node):
    s = get_option_value(node,'start-feature')
    return parse_feature_in_catalog(s)

############################################
# Analyze Morphology File ##################
############################################

def analyze_morph(morph_file_string, word_include_set=None):
    """
    Analyze the morph file in XTAG grammar.

    :param morph_file_string: The string read from the morph file
    :type morph_file_string: str
    :return: A dictionary repersenting the morph
    :rtype: dict(str,list(tuple(str,str,list(str))))

    This function does the parsing in the line order. Each line has a format like
    this,

    [index in morph] ([index in syntax] [POS in syntax] ([possible features])*)+

    in which the possible feature is the feature defined in the template file
    starting with '@'.

    The return value is a complex one. It is a dictionary using string (word in
    any form) as index, and the value is itself a list. The entries in the list
    some tuples indicating all possible morphs for that word. The tuple has three
    components, the first of which is the index in syntax, and the second is POS
    in syntax, while the third component is a list, which contains the features
    in the template.
    """

    morph = {}
    lines = morph_file_string.splitlines()
    for l in lines:
        # Filters out all blank lines using isspace() which filters out '\n' ' '
        if l.isspace() == True:
            continue
        index_pos_feature = l.split('#')
        # This is special
        m = index_pos_feature[0].split()
        # The first index_pos_feature is different since the first element after
        # split() is the index for the dictionary
        dict_key = m[0]

        if word_include_set is not None and dict_key not in word_include_set:
            continue

        morph_list = []  # To record multiple morphs in a line
        morph_list.append((m[1],m[2],m[3:]))
        for i in index_pos_feature[1:]:
            m = i.split()
            morph_list.append((m[0],m[1],m[2:]))
        morph[dict_key] = morph_list
        
    return morph
        

##############################################
# Analyze Syntax File ########################
##############################################

def check_index(index):
    if index == -1:
        raise TypeError('Invalid input line.')

def get_next_pair(s,start):
    # This function will extract next pair of <<XXXX>>OOOOO from the input string
    # s with a starting index = start. The return value is a tuple (entry,value,index)
    # The 3rd value is the next un-read position, could be -1 if there is no more
    # input
    index_start = s.find('<<',start)
    if index_start == -1:
        return (None,None,-1)
    index_end = s.find('>>',index_start) 
    check_index(index_end)
    entry = s[index_start + 2:index_end]
    start = index_end + 2
    index_start = s.find('<<',start)
    if index_start == -1:
        value = s[start:]
        index_return = len(s) - 1
    else:
        value = s[start:index_start]
        index_return = index_start

    return (entry,value,index_return)

def analyze_syntax(s):
    """
    A parser for the syntax file. A syntax file is indexed using the <<INDEX>>
    and the entries are made of several pairs of <<ENTRY>> and <<POS>>. Also
    we may have <<TREES>> and <<FAMILY>> in the syntax to specify the tree name
    or family name for that word. And finally, the <<FEATURES>> will give us
    some features about the word.

    Update: now we can also do tree-to-word searching, i.e. given a tree name or
    a family name, the dictionary can return a list of words which is applicable
    to the trees or tree families.
    
    :param s: A string read from the syntax file
    :type s: str

    :return: Two dictionaries enabling both word-to-trees and tree-to-words search
    :rtype: tuple(dict,dict)
    """
    # This function returns a dictionary, the index is exactly the <<INDEX>>
    # entry in the syntax file. Each keyword will fetch a list, the element of
    # which is just the lines with the same <<INDEX>>. Each list has four components
    # The first is called entry_list, the element of which is tuples with <<ENTRY>>
    # and <<POS>> being the 1st and 2nd element. The 2nd list is called tree_list
    # the element of which is tree name. The 3rd list is called family_list
    # the element of which is family name. The fourth list is called feature_list
    # the element of which is feature name.

    lines = s.splitlines()
    tokens = {}
    # This dictionary is used to use the tree names to do reverse query to get the
    # word or words
    reverse_trees = {}
    for l in lines:
        if l == '':
            continue
        next_start = 0
        entry_list = []
        tree_list = []
        family_list = []
        feature_list = []
        line_name = 'noname'
        while True:
            next_pair = get_next_pair(l,next_start)
            next_start = next_pair[2]
            entry = next_pair[0]
            value = next_pair[1]
            if next_start == -1:
                break
            elif entry == 'INDEX':
                line_name = value
            elif entry == 'ENTRY':
                entry_name = value
                next_pair = get_next_pair(l,next_start)
                next_start = next_pair[2]
                check_index(next_start)
                entry = next_pair[0]
                value = next_pair[1]
                if entry != 'POS':
                    raise TypeError('<<ENTRY>> must be followed by <<POS>>')
                else:
                    pos = value
                entry_list.append((entry_name,pos))
            elif entry == 'FAMILY':
                family_list = value.split()
            elif entry == 'TREES':
                tree_list = value.split()
            elif entry == 'FEATURES':
                feature_list = value.split()
            else:
                pass
                #raise TypeError('Unknown type: %s' % (entry))

        temp = (entry_list,tree_list,family_list,feature_list)
        if line_name in tokens:
            tokens[line_name].append(temp)
        else:
            tokens[line_name] = [temp]
            
    return tokens


def analyze_template(s):
    """
    Make feature structure templates using the lines in the template file. There
    are two kinds of templates, one of them is started by a @ mark, which is used
    in the morph file, and another type is started with a # mark, which is used
    in the syntax file. When we are constructing trees with a particular anchor,
    the code will search the templates of features and combine them with the xtag
    tree.

    :param s: A string read from the template file
    :type s: str
    :return: A pair of dictionaries, one for morph and another for syntax
    :rtype: tuple(dict,dict)
    """
    # The return value of this function is a tuple. The first element of the
    # tuple is a dictionary
    # using identifiers from morph.flat, and the entries are feature structures
    # with proper values set. The second element is a dictionary using 
    # keys from syntax-coded.flat, which will return a list containing 
    # all feature structures from a given identifier.
    lines = s.splitlines()
    feature_list = {}
    feature_list2 = {}
    for l in lines:
        #print l
        l = l.strip()
        if l == '' or l[0] == ';':
            continue
        
        index = l.find(';')
        if index != -1:
            l = l[:index]
            l = l.strip()
        
        if l[0] == '@':
            if l[-1] != '!':
                raise TypeError("Each line should be terminated with a '!'")
            l = l[1:-1]
            # we only split the name and the other part
            temp = l.split(None,1)
            name = temp[0]
            l = l[len(name):].strip()
            features = l.split(',')

            fs = FeatStruct()
            for f in features:
                f = f.strip()
                index = f.find('=')
                if f[0] == '@' and f[1:] in feature_list:
                    # unify() does not change in-place
                    fs = fs.unify(feature_list[f[1:]])    
                elif index != -1:
                    lhs = f[:index].strip()
                    #print lhs
                    rhs = f[index + 1:].strip()
                    # If not a reference then ref is by default True
                    ref = False
                    # rhs can also be reference
                    if rhs[0] == '@':      
                        rhs = feature_list[rhs[1:]]
                        ref = True # Used in make_fs
                    if lhs[0] != '<' or lhs[-1] != '>':
                        print(l)
                        raise TypeError('The left hand side of a feature structure must be wrapped with <>')
                    lhs = lhs[1:-1]
                    path = lhs.split()
                    #path.reverse()    # This method is in-place
                    #print "rhs = %s" % (rhs)
                    fs = fs.unify(make_fs(path,rhs,ref))
                else:
                    raise TypeError('Invalid line in template file.')
            feature_list[name] = fs
            #print name
            #print fs,'\n'
        elif l[0] == '#':
            if l[-1] != '!':
                raise TypeError('Invalid input line, must be terminated by "!"')
            l = l[1:-1]
            tokens = l.split(None,1)     # Split for once using space character
            word_pos = tokens[0].strip()

            features = tokens[1].split(',')
            new_fs = FeatStruct()
            for fs in features:
                tokens = fs.split(':',1)
                node_type = tokens[0].strip()
                tokens = tokens[1].split('=',1)
                lhs = tokens[0].strip()[1:-1]    # Remove <>
                rhs = tokens[1].strip()
                lhs = lhs.split()
                if node_type in new_fs:
                    #new_fs[node_type] = new_fs[node_type].unify(make_fs(lhs,rhs))
                    add_new_fs(new_fs[node_type],lhs,rhs) # Add the feature structure into an existing fs
                else:
                    new_fs[node_type] = make_fs(lhs,rhs)
                    
            if word_pos not in feature_list2:
                feature_list2[word_pos] = new_fs
            else:
                pass
                #print("duplicate definition", word_pos, feature_list2[word_pos])
                #raise KeyError('Duplicate defitinion detected: %s.' % (word_pos))
        else:
            raise TypeError('Cannot recognize line: %s.' % (l))
    return (feature_list,feature_list2)


# analyze_tree_1(s) will split the options and trees into a list, the first and second
# element of which is None, the third being the tree, and the fourth being
# the list of oprion, which will be processed in later phases
def analyze_tree_1(s):
    """
    Process the tree file for later use. On this stage we just separate the
    definition of tree structure and feature structures.
    
    :param s: The string read from a tree file
    :type s: str
    :return: An intermediate result
    :rtype: list
    """
    i = 0
    last_time = 0    # To record the position since the latest ')'
    stack = 0  # We use a simple stack to match brackets

    # Store final result, it is a list of list
    xtag_trees = []    
    options = None
    single_tree = None
    
    is_option = True # Distinguish between trees and feature structures
    for i in range(0,len(s)):
        if s[i] == '(':
            stack += 1
        elif s[i] == ')':
            stack -= 1
            if stack == 0 and is_option == True:
                option = s[last_time:i + 1]
                last_time = i + 1
                is_option = False
            elif stack == 0 and is_option == False:
                single_tree = s[last_time:i + 1]
                last_time = i + 1
                is_option = True
                
                # None is a placeholder for those added in later phases
                xtag_trees.append([None,None,single_tree.strip(),option.strip(),None,None])
    ############### Temporal Patch ##################
    trees = LL1.analyze_tree_file(s)
    for i in range(0,len(trees)):
        xtag_trees[i][5] = trees[i][0][1]
        
    return xtag_trees

def analyze_tree_2(xtag_trees):
    """
    Given the result of analyze_tree_1(), this function will further
    split the options string into several small strings, each is started by
    a ':' and ended by the start of another option or by the end of the string.

    :param xtag_trees: The result of analyze_tree_1
    :type xtag_trees: list
    :return: An intermediate result
    :rtype: list
    """
    for entry in xtag_trees:
        options_str = entry[3]
        name_start = options_str.find('"',0)
        name_end = options_str.find('"',name_start + 1)
        entry[0] = options_str[name_start + 1:name_end]

        option_length = len(options_str)
        option_start = name_end + 1
        option_end = option_start
        options_list = []

        # state = 0: outside any structure
        # state = 1: in an option
        # state = 2: in brackets
        # state = 3: in quotes
        state = 0
        
        # One character less, because we need to pre-fetch a character
        for i in range(name_end + 1,option_length - 1):
            ch = options_str[i]
            ch_forward = options_str[i + 1] # pre-fetch
            ch_backward = options_str[i - 1] # back_fetch
            
            if state == 0 and (ch == '\n' or ch == ' '):
                continue
            elif state == 0 and ch == ':':
                state = 1
                option_start = i + 1
            # We should avoid "(:" being considered as an ending symbol
            elif state == 1 and ch != '"' and ch != '(' and ch_forward == ':':
                state = 0
                option_end = i
                options_list.append(options_str[option_start:option_end])
            # This must happen as the last state transition
            elif state == 1 and ch_forward == ')': 
                options_list.append(options_str[option_start:i + 1])
            # skip everything between '(' and ')'
            elif state == 1 and ch == '(':
                state = 2
            elif state == 2 and ch == ')':
                state = 1
            # skip everything between '"' and '"', except '\"'
            elif state == 1 and ch == '"':
                state = 3
            elif state == 3 and ch == '"': # We must distinguish between '\"' and '"'
                if ch_backward != '\\':
                    state = 1
                    
        entry[3] = options_list
        
    return xtag_trees

def analyze_tree_3(xtag_trees):
    """
    Given the result of analyze_tree_2(), this function will extract
    the feature structure specifications into a separate list, and then extract
    RHS as well as LHS for further use.

    :param xtag_trees: The result of analyze_tree_2
    :type xtag_trees: list
    :return: An intermediate result
    :rtype: list
    """
    pattern = "UNIFICATION-EQUATIONS"
    pattern_len = len(pattern)

    for entry in xtag_trees:
        features = []
        f_temp = None
        
        for option in entry[3]:
            if option[0:pattern_len] == pattern:
                quote_start = option.find('"')
                quote_end = option.find('"',quote_start + 1)
                f_temp = option[quote_start + 1:quote_end]
                break
        else:
            raise NameError("Cannot find unification specification.")

        f_temp = f_temp.splitlines()
        for i in f_temp:
            i = str.strip(i)
            if i != '':
                exp = i.split('=')
                exp[0] = exp[0].strip()
                exp[1] = exp[1].strip()
                features.append(exp)
                
        entry[1] = features
    return xtag_trees


# This function is specially designed for processing the format used in .trees files, given the string indicating
# a path in the format like PP.b:<assign-case other> it will return a list [PP.b, assign-case, other]
def get_path_list(s):
    colon = s.find(':')
    if colon == -1:
        raise ValueError("No colon found in the path given.")
    left = s[:colon]
    right = s[colon + 2:-1]
    # Some features may not have a '.b' or '.t' explicitly so we assume that
    # there is a '.b' by default
    if left.find('.') == -1:
        left += '.b'
    right = [left] + right.split(' ')
    return right
    

def analyze_tree_4(xtag_trees):
    """
    Given the result of analyze_tree_3(), this function will make
    use of FeatStruct, and build a feature structure dictionary.

    :param xtag_trees: The result of analyze_tree_3
    :type xtag_trees: list
    :return: An intermeidate result
    :rtype: list
    """
    for xtag_entry in xtag_trees:
        features =  {}
        for feature_entry in xtag_entry[1]:
            lhs = feature_entry[0]
            rhs = feature_entry[1]
            #l_separator = lhs.find(':')
            r_separator = rhs.find(':')
            
            # We do not process reference until the fifth pass
            # and reference is identified by a ':' on RHS
            if r_separator == -1:
                lhs_list = get_path_list(lhs)
                add_new_fs(features,lhs_list,rhs)

        xtag_entry[4] = features
    
    return xtag_trees

def analyze_tree_5(xtag_trees):
    """
    Given the result of analyze_tree_4(), this function will continue
    to build the feature structure, and in this phase we must add all values
    even if they are not defined by the tree grammar.

    :param xtag_trees: The result of analyze_tree_4
    :type list:
    """
    for xtag_entry in xtag_trees:
        features = xtag_entry[4]
        for feature_entry in xtag_entry[1]:
            lhs = feature_entry[0]
            rhs = feature_entry[1]
            l_separator = lhs.find(':')
            r_separator = rhs.find(':')   

            if r_separator != -1:
                # get_path_list will return a list, the first item of which
                # is the node name, and the remaining part is the path,
                # e.g. a:<b c d> will get the result as [a,b,c,d]
                lhs_list = get_path_list(lhs)
                rhs_list = get_path_list(rhs)
                rhs_value = features
                # If the path exists till the end, then this will keep True
                # but if we cannot find the key on some level then
                # this will be False
                found_key = True
                for i in rhs_list:
                    if i not in rhs_value:
                        rhs_value[i] = FeatStruct()
                        rhs_value = rhs_value[i]
                        found_key = False
                    else:
                        rhs_value = rhs_value[i]

                if found_key == False:
                    rhs_value['__or_'] = ''

                add_new_fs(features,lhs_list,rhs_value,1)

    return xtag_trees


def parse_from_files(cata, files, language):
    """
    Get the TAGTreeSet from the gram file. The file format
    is defined in Upenn Xtag project.
    """
    if not isinstance(files, str):
        raise TypeError('input should be a base string')

    tree_list = get_file_list(cata, files)
    tagset = TAGTreeSet()
    tagset[files] = TAGTreeSet()

    file_names = tree_list[0]
    directory = tree_list[1]

    for fn in file_names:
        path = os.sep.join(['xtag_grammar', language, directory, fn])
        text = open(nltk.data.find(path), 'r').read()
        tagset[files][fn] = TAGTreeSet()
        tagset[files][fn] += grammar_file_parse(text)

    return tagset

def grammar_file_parse(text):
    """
    Get a TAGTreeSet from grammar files

    :param text is a string describing TAG trees, the form is defined in
    UPenn Xtag project
    """
    calist = analyze_tree_3(analyze_tree_2(analyze_tree_1(text)))
    arglist = analyze_tree_5(analyze_tree_4(calist))
    tagset = TAGTreeSet()      
    for element in arglist:
        new_tree = parse_tree_list(element[2], element[4])
        tree_name = element[0]
        new_tree.set_comment(element[5]['COMMENTS'])
        tagset[tree_name] = new_tree

        # Manually set tree name on each node in tree
        for s in new_tree.subtrees():
            s.tree_name = tree_name

    return tagset

def parse_tree_list(text, fs):
    tree_list = parse_brackets(text)
    t = build_tree(tree_list[0], fs)
    return t

def parse_brackets(text):
    text = '(' + text + ')'
    lresult = []
    ss = ''
    for c in text:
        if c == '(':
            if len(ss) != 0:
                lresult.append(lresult.pop() + ss.split())
                ss = ''
            lresult.append([])
        elif c == ')':
            last = lresult.pop()
            if len(ss) != 0:
                last += ss.split()
                ss = ''
            if len(lresult) > 0:
                lresult.append(lresult.pop() + [last])
            else:
                lresult.append(last)
        else:
            ss += c
    return lresult[0]

def build_tree(tree_list, fs):
    node = parse_node(tree_list[0])
    node_name = node[0]
    node_attr = node[1]
    top_fs = FeatStruct()
    bot_fs = FeatStruct()
    if node_name + '.t' in fs:
        top_fs = fs[node_name + '.t']
    if node_name + '.b' in fs:
        bot_fs = fs[node_name + '.b']
    if len(tree_list) == 1:
        return TAGTree((node_name, top_fs, bot_fs), [], node_attr)
    else:
        sub_tree = []
        for i in range(1, len(tree_list)):
            sub_tree.append(build_tree(tree_list[i], fs))
        return TAGTree((node_name, top_fs, bot_fs), sub_tree, node_attr)
 
def parse_node(node_list):
    clist = []
    pre = node_list[0][0][0]
    suf = node_list[0][0][2]
    pre = pre.replace('"', '')
    suf = suf.replace('"', '')
    for i in range(0,2):
        if len(node_list) > 1:
            clist = [(node_list[2*j - 1], node_list[2 * j]) 
                      for j in range(1, int(len(node_list)/2 + 1))]
    if ord(pre[0]) == 6 and len(pre) == 1:
        pre = '\u03b5'
    
    if len(suf) != 0:
        pre = pre + '_' + suf
    if (':substp', 'T') in clist:
        attr = 'subst'
    elif (':headp', 'T') in clist:
        attr = 'head'
    elif (':footp', 'T') in clist:
        attr = 'foot'
    else:
        attr = None
    return (pre, attr, clist)
            

######################################
# Word and feature conversion ########
######################################

def check_pos_equality(morph_pos,syntax_pos):
    """
    Check whether two POS tags from morph and syntax respectively are equal.

    We have a mapping relation file recording all possible mappings from the
    tags in the syntax to the names in the morph.

    :param morph_pos: The pos tag in morph
    :type morph_pos: str
    :param syntax_pos: The pos tag in syntax
    :type morph_pos: str
    :return: True if they are equivalent, False if not
    :rtype: bool
    """
    if morph_pos == syntax_pos:  # Check literal equality first
        return True
    elif syntax_pos in dicts[4]: # Then check mapping equality
        return dicts[4][syntax_pos] == morph_pos
    else:  # Else not equal
        return False
    
def make_pos_mapping(s):
    """
    Make a dictionary mapping the pos tag in syntax to the pos tag in the morph.
    The mapping relation is coded using the following form:

    [POS in syntax] -> [POS in morph] (, [another POS])*

    :param s: The content of the mapping file
    :type s: str
    :return: A dictionary repersenting the mapping relation
    :rtype: dict
    """
    lines = s.splitlines()
    mapping = {}
    for l in lines:
        if l.strip() == '':
            continue
        lr = l.split('->')
        if len(lr) != 2:
            raise ValueError('Not a valid line in the mapping file: %s\n' % (l))
        lr[0] = lr[0].strip()
        lr[1] = lr[1].strip()
        if lr[0] in mapping:
            raise KeyError('Key %s already exists!' % (lr[0]))
        mapping[lr[0]] = lr[1]

    return mapping

