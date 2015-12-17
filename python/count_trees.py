import glob, json, nltk, re
from collections import defaultdict, deque

def label_to_tree_word_loc(label):
    tree_name = re.search('(.*)\[', label).group(1)
    word = re.search('\[(.*?)\]', label).group(1)
    loc_match = re.search('\<(.*?)\>', label)
    loc = loc_match.group(1) if loc_match is not None else None
    return tree_name, word, loc

def process_deriv_tree(deriv_tree, global_counts, context_sensitive_counts):
    queue = deque([(None, deriv_tree)])
    while len(queue) > 0:
        parent, d_tree = queue.popleft()

        if isinstance(d_tree, nltk.Tree):
            tree_label = d_tree.label()
            queue += [(d_tree, c) for c in d_tree]
        elif isinstance(d_tree, str):
            tree_label = d_tree
        else:
            raise ValueError

        if parent is None:
            p_tree_name = None
        else:
            p_tree_name, p_word, p_loc = label_to_tree_word_loc(parent.label())

        tree_name, word, loc = label_to_tree_word_loc(tree_label)
        global_counts[tree_name] += 1
        context_sensitive_counts[(p_tree_name, loc, tree_name)] += 1

def count_corpus(treedir='parse_trees', count_file='tree_counts.txt'):
    # just stores counts of each tree of form {'treename': count}
    global_counts = defaultdict(int)
    # stores context sensitive counts of each tree of form {('parent_treename', 'action location', 'treename'): count}
    context_sensitive_counts = defaultdict(int)

    for treefile in glob.glob(treedir + '/*.txt'):
        tree_dict = json.load(open(treefile, 'r'))
        deriv_tree_str = tree_dict['deriv']
        deriv_tree = nltk.Tree.fromstring(deriv_tree_str)
        process_deriv_tree(deriv_tree, global_counts, context_sensitive_counts)

    counts_dict = {
        'global_counts': dict(global_counts), 
        'context_sensitive_counts': dict(context_sensitive_counts)
    }

    with open(count_file, 'w') as f:
        f.write(str(counts_dict))

def investigate_counts(count_file='tree_counts.txt'):
    with open(count_file, 'r') as f:
        counts_dict = eval(f.read())

    global_counts = counts_dict['global_counts']
    for tree, count in sorted(global_counts.items(), key=lambda x: x[1], reverse=True):
        print(tree, count)

def have_tree_semantics(deriv_tree, tree_set):
    queue = deque([deriv_tree])
    while len(queue) > 0:
        d_tree = queue.popleft()

        if isinstance(d_tree, nltk.Tree):
            tree_label = d_tree.label()
            queue += [c for c in d_tree]
        elif isinstance(d_tree, str):
            tree_label = d_tree
        else:
            raise ValueError

        tree_name, word, loc = label_to_tree_word_loc(tree_label)
        if tree_name not in tree_set:
            return False
    return True

def parse_deriv_semantics(treedir='parse_trees'):
    tree_set = set([
        "alphaNXN", # 18954
        "betaDnx", # 5663
        "betaNn", # 5156
        "betasPU", # 4607
        "betaVvx", # 3500
        "betaAn", # 2937
        "betanxPnx", # 1797
        "alphanx0Vnx1", # 1577
        "alphanx1V", # 1018
        "alphanx0V", # 205
        "betavxPnx", # 1636
        "betasPUs", # 929
        "betanxPUnx", # 876
        "betaARBvx", # 766
        "betanx0Vs1", # 754
        "alphanx0BEnx1", # 570
        "alphanx0Pnx1", # 535
        "betanxGnx", # 500
        "betaVs", # 496
        "alphanx0Vnx1-PRO", # 470
        "betaCOMPs", # 388
        "betanxPUs", # 384
        #"betanx1CONJnx2", # 384
        "alphanx0N1", # 375
    ])
    count = 0
    for treefile in glob.glob(treedir + '/*.txt'):
        tree_dict = json.load(open(treefile, 'r'))
        deriv_tree_str = tree_dict['deriv']
        deriv_tree = nltk.Tree.fromstring(deriv_tree_str)
        parse_tree_str = tree_dict['parse']
        parse_tree = nltk.Tree.fromstring(parse_tree_str)

        if have_tree_semantics(deriv_tree, tree_set):
            count+=1
            if count > 1:
                deriv_tree.draw()
                print(str(deriv_tree))
                parse_tree.draw()
                exit()

def count_semantic_parses(treedir='parse_trees'):
    count = 0
    tree_set = set([
        "alphaNXN", # 18954
        "betaDnx", # 5663
        "betaNn", # 5156
        "betasPU", # 4607
        "betaVvx", # 3500
        "betaAn", # 2937
        "betanxPnx", # 1797
        "alphanx0Vnx1", # 1577
        "alphanx1V", # 1018
        "alphanx0V", # 205
        "betavxPnx", # 1636
        "betasPUs", # 929
        "betanxPUnx", # 876
        "betaARBvx", # 766
        "betanx0Vs1", # 754
        "alphanx0BEnx1", # 570
        "alphanx0Pnx1", # 535
        "betanxGnx", # 500
        "betaVs", # 496
        "alphanx0Vnx1-PRO", # 470
        "betaCOMPs", # 388
        "betanxPUs", # 384
        #"betanx1CONJnx2", # 384
        "alphanx0N1", # 375
        #"betas1CONJs2", # 373
        #"alphanx0Ax1", # 371
        #"betavxPs", # 359
        #"betavxARB", # 345
        #"betaN1nx1V", # 342
        #"alphanx1V-PRO", # 340
        #"alphaN1s0", # 279
        #"alphas0N1", # 274
        #"betaN0nx0Vnx1", # 273
        #"betaPUs", # 239
        #"alphaN", # 225
        #"betan1CONJn2", # 225
        #"alphaPu", # 224
        #"betaN0nx0Pnx1", # 209
        #"betaARBs", # 190
        #"alphas0Ax1", # 185
        #"alphanx2Vnx1", # 180
        #"betaN0nx0N1", # 165
        #"betaARBa", # 165
        #"betanx0Vs1-PRO", # 153
        #"alphaD", # 146
        #"betaNPnx1nx0Pnx1", # 144
        #"betaPnxs", # 141
        #"alphaXGnx0Vs1", # 140
        #"betaN1nx0Pnx1", # 125
        #"betaCONJs", # 122
        #"alphaNXnxG", # 114
        #"alphanx0N1-PRO", # 109
        #"alphanx0Ax1-PRO", # 109
        #"alphaDnx0V", # 108
        #"alphaNXNs", # 105
        #"alphanx0Pnx1-PRO", # 104
        #"betasPUnx", # 103
        #"betaNEGvx", # 102
        #"alphaPXPnx", # 100
        #"betaN0nx0V", # 85
        #"alphanx0V-PRO", # 84
        #"betaN0nx0Ax1", # 83
        #"alphaXW0nx0Vs1", # 78
        #"betanxN", # 77
        #"betaPss", # 75
        #"betaN0nx0Vs1", # 68
        #"betaVergativen", # 66
        #"alphaP", # 64
        #"alphanx0Vnx2nx1", # 62
        #"betaCnxPnx", # 56
        #"betaN1nx0Vnx1", # 55
        #"alphanx0Vplnx1", # 53
        #"betaXInx0Vs1", # 52
        #"betanx1Vs2", # 52
        #"betaXnx0Vs1", # 48
        #"alphaDnx0Vs1", # 47
        #"alphaW0nx0Vnx1", # 45
        #"betaspuPs", # 45
        #"betanxARB", # 45
        #"betaa1CONJa2", # 44
        #"betaENc1nx1V", # 44
        #"betavxN", # 43
        #"betaARBnx", # 43
        #"betaXNc0nx0Vs1", # 42
        #"alphaA", # 42
        #"alphanx2Vnx1-PRO", # 41
        #"alphapW1nx0Pnx1", # 40
        #"betanx0Vnx1s2", # 40
        #"alphaW1nx1V", # 34
        #"betaARBpx", # 32
        #"alphaEW1nx1V", # 32
        #"betaXNcnx0Vs1", # 29
        #"alphaAXA", # 28
        #"betaARBd", # 27
        #"betaspuARB", # 26
        #"betaARBarb", # 25
        #"betaspuPnx", # 25
        #"alphanx0VPnx1", # 24
        #"alphanx1Vpnx2", # 23
        #"alphas0Vnx1", # 22
        #"alphanx0Vax1", # 22
        #"betanx1Vs2-PRO", # 22
        #"alphanx0Px1", # 21
        #"alphanx1Vpl", # 20
        #"alphanx0Vplnx1-PRO", # 20
        #"betaNpxnx1V", # 20
        #"betaNvx", # 20
        #"betaNs", # 19
        #"betaVvx-adj", # 19
        #"betapuARBpuvx", # 19
        #"betanxPs", # 18
        #"betaCARBa", # 17
        #"betaCARBarb", # 17
        #"betanxVpus", # 17
        #"betapunxVpuvx", # 16
        #"alphaW0nx0Pnx1", # 15
        #"betaNpxnx0Vnx1", # 15
        #"alphanx0Vnx1Pnx2", # 14
        #"betaN1nx1Vpl", # 14
        #"alphanx0Vnx2nx1-PRO", # 14
        #"alphaDnxG", # 14
        #"betaNpxs0N1", # 14
        #"alphaW1nx0Vnx1", # 13
        #"alphanx0Vnx1pnx2", # 13
        #"betanxARBs", # 12
        #"betapx1CONJpx2", # 11
        #"betaspuVnx", # 11
        #"alphanx1Vpl-PRO", # 11
        #"alphanx1VPnx2", # 11
        #"alphanx0Vnx1pl", # 11
        #"alphaW0s0N1", # 10
        #"betavPU", # 10
        #"betanxP", # 10
        #"alphaW0nx0V", # 10
        #"betaN1nx2Vnx1", # 10
        #"alphaW1nx0Pnx1", # 9
        #"betaaxPnx", # 9
        #"betaN0nx0Px1", # 9
        #"betaNpxnx0N1", # 8
        #"alphanx0N1s1", # 8
        #"alphanx0VPnx1-PRO", # 8
        #"alphaW1nx2Vnx1", # 8
        #"alphaW0nx0Vs1", # 8
        #"betaN2nx2Vnx1", # 8
        #"betaNEGa", # 8
        #"betaNpxnx0Ax1", # 6
        #"betaN0nx0VPnx1", # 6
        #"betaNpxs0Ax1", # 6
        #"alphas0Vs1", # 6
        #"betapuPpuvx", # 6
        #"alphanx0Vpl-PRO", # 6
        #"betaN0nx0Vplnx1", # 6
        #"alphanx0Vpl", # 6
        #"betaENcnx1V", # 6
        #"betaN0nx0Vnx2nx1", # 6
        #"alphaAV", # 6
        #"betaXnx0Vs1-PRO", # 6
        #"alphaW0s0Ax1", # 5
        #"alphaAd", # 5
        #"betaN1nx1Vs2", # 5
        #"alphaEnx1V", # 5
        #"betaEN1nx1V", # 5
        #"alphanx0Vpnx1", # 5
        #"betaN1nx1Vpnx2", # 5
        #"betaNpxnx0Pnx1", # 5
        #"betapuVnxpuvx", # 5
        #"betaNpxnx2Vnx1", # 5
        #"betaN2nx0Vnx2nx1", # 5
        #"betanxPUa", # 4
        #"alphaW1nx0Vs1", # 4
        #"alphanx0Px1-PRO", # 4
        #"alphaW0nx0Vplnx1", # 4
        #"alphaW0nx0Vnx2nx1", # 4
        #"betavxDN", # 4
        #"alphanx0lVN1", # 4
        #"betadD", # 4
        #"betaN0nx0Vnx1s2", # 4
        #"betaaARB", # 4
        #"alphanx1VPnx2-PRO", # 4
        #"alphanx1VP", # 4
        #"betaNpxnx0V", # 4
        #"alphanx0Vax1-PRO", # 3
        #"betaARBarbs", # 3
        #"betad1CONJd2", # 3
        #"betanxnxARB", # 3
        #"alphanx1Vpnx2-PRO", # 3
        #"betap1CONJp2", # 3
        #"alphaW1nx0Vnx2nx1", # 3
        #"betaN0nx0Vnx1pnx2", # 3
        #"betavxP", # 3
        #"alphaW1nx0N1", # 3
        #"alphanx0Vnx1pnx2-PRO", # 3
        #"betaN0nx0Vpl", # 3
        #"betanx0Vnx1s2-PRO", # 3
        #"betaXN0nx0Vs1", # 3
        #"betaNpxnx0Vs1", # 3
        #"alphaPW1nx0Px1", # 3
        #"alphaEnx1V-PRO", # 3
        #"betaN2nx0Vnx1pnx2", # 2
        #"betaax1CONJax2", # 2
        #"alphaW0nx0Ax1", # 2
        #"alphaW0s0Vnx1", # 2
        #"betavpunxVpu", # 2
        #"betaspunxV", # 2
        #"betavxnxARB", # 2
        #"alphaW2nx0Vnx1s2", # 2
        #"alphaW2nx0Vnx2nx1", # 2
        #"alphanx0Vpnx1-PRO", # 2
        #"alphaW0nx0Px1", # 1
        #"betaN2nx0Vnx1Pnx2", # 1
        #"betaN0nx0Vax1", # 1
        #"betaNpxItVad1s2", # 1
        #"betaNpxnx0Vax1", # 1
        #"alphaREW1nx1VPnx2", # 1
        #"alphaDnx0Vpl", # 1
        #"betaN1nx1VP", # 1
        #"betaDNpx", # 1
        #"betavxDA", # 1
        #"betapPU", # 1
        #"alphaW1nx1Vpl", # 1
        #"alphaW1ItVnx1s2", # 1
        #"alphas0V", # 1
        #"betaARBPnxs", # 1
        #"betaN0nx0Vnx1pl", # 1
        #"alphanx1VP-PRO", # 1
        #"alphaRW1nx1VA2", # 1
        #"betaN2nx1Vpnx2", # 1
        #"alphaW1nx0Vplnx1", # 1
        #"betaN1nx1VPnx2", # 1
        #"betaarb1CONJarb2", # 1
        #"alphaDnx0VPnx1", # 1
        #"betanx1CONJARBnx2", # 1
        #"betapunxVnx1puvx", # 1
        #"betanARB", # 1
        #"alphaW0nx0N1", # 1
        #"alphaAXAs", # 1
        #"alphanx0Vnx1Pnx2-PRO", # 1
        #"alphanx1Vp-PRO", # 1
        #"betaARBpx1CONJpx2", # 1
    ])
    for treefile in glob.glob(treedir + '/*.txt'):
        tree_dict = json.load(open(treefile, 'r'))
        deriv_tree_str = tree_dict['deriv']
        deriv_tree = nltk.Tree.fromstring(deriv_tree_str)
        if have_tree_semantics(deriv_tree, tree_set):
            count += 1
    print('Number of trees with semantic parses: ', count)

if __name__ == '__main__':
    #count_corpus()
    #investigate_counts()
    #count_semantic_parses()
    parse_deriv_semantics()
