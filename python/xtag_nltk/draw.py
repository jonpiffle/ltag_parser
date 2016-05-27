# Natural Language Toolkit: Tree-Adjoining Grammar
#
# Copyright (C) 2001-2013 NLTK Project
# Author: WANG Ziqi, Haotian Zhang <{zwa47,haotianz}@sfu.ca>
#
# URL: <http://www.nltk.org/>
# For license information, see LICENSE.TXT
#

import os
import copy

### FOR SOME REASON ORDER IS IMPORTANT AND CAN MAKE dicts = None IF WRONG ###
from nltk.tree import *
from nltk.util import in_idle
from nltk.draw.util import *
from nltk.draw.tree import (TreeView, TreeWidget, TreeSegmentWidget)
from nltk.featstruct import FeatStruct
from xtag_nltk.tagtree import TAGTree, TAGTreeSet
from xtag_nltk.grammar import Grammar
from tkinter import *
from tkinter import ttk
from math import sqrt, ceil

class LexView(object):
    """
    A window that lexicalize a TAG Tree set. When type in
    word in the text entry, the TAG Tree Set will be lexicalized.
    Only TAG Trees that can be lexicalized with that word
    remain in the window. The word will be attached to the 
    substitution node of the tree, and feature structure
    will be updated to make the tree consistent with the word.

    """

    def __init__(self, grammar):
        self._top = Tk()
        self._top.title('NLTK')
        self._top.bind('<Control-x>', self.destroy)
        self._top.bind('<Control-q>', self.destroy)
        self._top.geometry("1400x800")

        self.frame = Frame(self._top)

        v = StringVar()
        self._e = Entry(self.frame, textvariable=v)
        self._e.bind("<Return>", self.return_pressed)
        all_button = Button(self.frame, text='Show All', command=self.show_all)
        button = Button(self.frame, text="Search", command=self.lex_search)
        self._e.pack(expand=1, fill='both', side=LEFT)

        self.phrases = StringVar()
        self.phrases.set("")
        update_button = Button(self.frame, text="Select", command=self.update_lex_tree)
        self._w = OptionMenu(self.frame, self.phrases, [])
        update_button.pack(side=RIGHT)
        self._w.pack(side=RIGHT)
        wl = Label(self.frame, text='Anchors:')
        wl.pack(side=RIGHT)
        
        all_button.pack(side=RIGHT)
        button.pack(side=RIGHT)
        self.frame.pack(fill='both')

        self.grammar = grammar
        self._tagset = grammar.trees
        
        self._treeview = TAGTreeSetView(self._tagset, (self._top, self.display))
        self._treeview.pack()
        self._dicword = {}
        self._count = {}

    def display(self, event=None):
        """
        Display the tag tree on the canvas when the tree
        is selected.
        """
        node = self._treeview._tagview.focus()
        path = self._treeview._tagview.set(node, "fullpath").split('/')
        tree = self._treeview._trees
        for subpath in path[1:]:
            if subpath in tree:
                if not isinstance(tree, type(self._treeview._trees)):
                    if tree._lex:
                        tree[subpath] = tree[subpath].copy(True)

                tree = tree[subpath]
            else:
                raise TypeError("%s: tree does not match"
                             % type(self).__name__)

        if not isinstance(tree, type(self._treeview._trees)):
            # Words is supposed to be all words with associated with a tree
            words = []

            if len(words) > 0:
                self.update_anchor(words)
            if tree.start_feat:
                self._treeview._sfs_button['text'] = 'Delete Start Features'
            else:
                self._treeview._sfs_button['text'] = 'Add Start Features'
            if tree._lex:
                tree.lexicalize()
                tree._lex = False
                self._treeview._tw.redraw(self._treeview._show_fs, tree)
            else:
                self._treeview._tw.redraw(self._treeview._show_fs, tree)
        else:
            for path in tree:
                if isinstance(tree[path], type(self._treeview._trees)):
                    return
            self.phrases.set("")
            if isinstance(tree, type(self._treeview._trees)) and subpath[-6:] == '.trees':
                tree_fam = tree[subpath]
                treename = subpath[:-6]
            else:
                treename = subpath

    def update_anchor(self, words):
        """
        Update anchors with selected words
        :param: selected words
        :type: basestring
        """
        self.clean_anchor()
        for choice in words:
            phrase = ''
            for term in choice:
                phrase = phrase + term[0] + ' '
            if not phrase in self._dicword:
                self._dicword[phrase] = choice
            else:
                continue
            import tkinter as tk
            self._w['menu'].add_command(label=phrase, command=tk._setit(self.phrases, phrase))

    def clean_anchor(self):
        """
        Clean anchors of this viewer
        """
        self._dicword = {}
        self._w['menu'].delete(0, 'end')        

    def update_lex_tree(self):
        """
        Update the viewer with selected words
        """
        words = self.phrases.get().split()
        if len(words) == 0:
            return
        self._tagset = TAGTreeSet()
        self._e.delete(0, END)
        self._treeview.clear()
        for word in words:
            self._tagset[word] = TAGTreeSet()
            fset = self._tagset[word]
            self.lexicalize_tagset(lex_list, fset)
            for morph in lex_list:
                if len(morph[0]) > 1:
                    phrases = [v[0] for v in morph[0]]
                    if phrases == words:
                        index = ''
                        for i in phrases:
                            index = index + i + ' '
                        if not index in self._tagset:
                            self._tagset[index] = TAGTreeSet()
                        fset = self._tagset[index]
                        self.lexicalize_tagset([morph], fset)
        self._tagset.set_start_fs(self._alltrees.start_fs)
        self._treeview.update(self._tagset)
        self._count = {}

    def return_pressed(self, event):
        """
        Short-cut for pressing return to show feature structures
        """
        self.clean_anchor()
        words = self._e.get().split()
        if len(words) == 0:
            self.show_all()
            return
        self.lex_search()

    def resize(self, *e):
        """
        Resize the window
        """
        self._treeview(e)

    def show_all(self):
        """
        Show all the TAG Tree in window
        """
        self.clean_anchor()
        self._e.delete(0, END)
        self._tagset = self._alltrees
        self._treeview.clear()
        self._treeview.update(self._tagset)
        return
    
    def lex_search(self):
        """
        Update the viewer after lexicalization
        """
        words = self._e.get().split()
        if len(words) == 0:
            return
        self.clean_anchor()
        self._tagset = TAGTreeSet()
        self._treeview.clear()
        for word in words:
            self._tagset[word] = TAGTreeSet()
            fset = self._tagset[word]
            lex_list = word_to_features(word)
            self.lexicalize_tagset(lex_list, fset)
            for morph in lex_list:
                if len(morph[0]) > 1:
                    phrases = [v[0] for v in morph[0]]
                    if words == phrases:
                        index = ''
                        for i in phrases:
                            index = index + i + ' '
                        if not isinstance(self._tagset[index], TAGTreeSet):
                            self._tagset[index] = TAGTreeSet()
                        fset = self._tagset[index]
                        self.lexicalize_tagset([morph], fset)
        self._tagset.set_start_fs(self._alltrees.start_fs)
        self._treeview.update(self._tagset)
        self._count = {}

    def lexicalize_tagset(self, lex_list, fset):
        """
        Lexicalize all TAG tree set
        :param lex_list: a list of lexicalized node
        :type: list
        :alltrees: TAG tree set to be lexicalized
        :type: TAGTreeSet
        """
        for morph in lex_list:
            index = ''
            for i in morph[0]:
                index = index + i[0] + '.' + i[1] + ' '
            for i in morph[5][1]:
                index = index + i + '_'
            index = index[:-1]
            if index not in fset:
                fset[index] = TAGTreeSet()
            sset = fset[index]
            if len(morph[2]) > 0:
                for tf in morph[2]:
                    ckey = index+tf
                    tf = tf + '.trees'
                    if tf not in sset:
                        key = tf
                        self._count[ckey] = 0
                    else:
                        key = tf[:-5] + '_' + str(self._count[ckey]) + '.trees'
                    sset[key] = TAGTreeSet()
                    index = None
                    for sub in self._alltrees:
                        if tf in self._alltrees[sub]:
                            index = sub
                    if not index:
                        raise NameError('No tree family')
                    sset[key] += self._alltrees[index][tf].copy(True)
                    for t in sset[key]:
                        if sset[key][t]._lex:
                            sset[key][t]._lex_fs
                        sset[key][t].init_lex(morph[0], morph[3], morph[4])
                    self._count[ckey] += 1
            else:
                for t in morph[1]:
                    ckey = index+t
                    if t not in sset:
                        key = t
                        self._count[ckey] = 0
                    else:
                        key = t + '_' + str(self._count[ckey])
                    for sub in self._alltrees:
                        for tr in self._alltrees[sub]:
                            if t in self._alltrees[sub][tr]:
                                sset[key] = self._alltrees[sub][tr][t].copy(True)
                    self._count[ckey] += 1
                    if not isinstance(sset[key], TAGTree):
                        raise TypeError('Not TAGTree')
                    sset[key].init_lex(morph[0], morph[3], morph[4])

    def destroy(self, *e):
        if self._top is None: return
        self._top.destroy()
        self._top = None

    def mainloop(self, *args, **kwargs):
        if in_idle(): return
        self._top.mainloop(*args, **kwargs)

class TAGTreeView(TreeView):
    """
    A graphical diagram that displays a single TAG Tree, inherit from 
    ``TreeView``. ``TAGTreeView`` manages a group of ``TAGTreeWidget``
    that are used to display a TAG Tree. When read another
    tree to display, the ``TAGTreeWidget`` will be updated.

    """
    def __init__(self, trees, **kwargs):
        if "parent" in kwargs:
            self._top = kwargs["parent"]
        else:
            self._top = Tk()
            self._top.title('NLTK')
            self._top.bind('<Control-p>', lambda e: self.print_to_file())
            self._top.bind('<Control-x>', self.destroy)
            self._top.bind('<Control-q>', self.destroy)

        self._cframe = CanvasFrame(self._top)
        self._widgets = []
        self.treecomment = {}
        self.oldcomment = None
        self.show_fs = True
        self._trees = trees
        self.redraw(True, trees)


    def _init_menubar(self):
        """
        Hide menubar
        """
        pass

    def clear(self):
        """
        Clean canvas
        """
        self.redraw(False)

    def collapse_comment(self):
        """
        Show comment of selected TAG tree
        """
        length = len(self._trees[0].leaves())
        width = pow(length, 1.5)*80 + 230 + pow(len(self._trees[0]), 2)*35
        self._cframe.add_widget(self.treecomment[id(self._trees[0])].widget(),
                                width, 0)

        trees = self._trees
        show = self.show_fs
        for i in self._widgets:
            self._cframe.destroy_widget(i)
        self._sizeco = IntVar(self._top)
        self._size.set(12)
        bold = ('helvetica', -self._size.get(), 'bold')
        helv = ('helvetica', -self._size.get())


        self._width = int(ceil(sqrt(len(trees))))
        self._widgets = []
        for i in range(len(trees)):
            if isinstance(trees[i], TAGTree):
                fs = trees[i].get_all_fs()
            else:
                fs = None
            widget = TAGTreeWidget(self._cframe.canvas(), trees[i], fs,
                                show, self._top, leaf_color='#008040',
                                roof_color='#004040', roof_fill='white',
                                line_color='#004040', leaf_font=helv, 
                                **self.attribs)
            widget['yspace'] = 70
            widget['xspace'] = 70
            self._widgets.append(widget)
            self._cframe.add_widget(widget, 0, 0)
        
        self._layout()
        self._cframe.pack(expand=1, fill='both', side = LEFT)

    def redraw(self, show, *trees, **attribs):
        """
        Update the current canvas to display another tree, set te feature
        sturctures to display or hidden.

        :param show: a boolean value for whether to show the feature
        sturcture on the canvas
        :type: bool
        :param trees: a list of tree segments
        :type: list
        """ 
        self._trees = trees
        self.attribs = attribs
        if trees and isinstance(trees[0], TAGTree): 
            if self.oldcomment:
                self._cframe.destroy_widget(self.oldcomment.widget()) 
            self.treecomment[id(trees[0])] = CommentWidget(self._cframe.canvas(),
                                                           self,width=500)
            self.oldcomment = self.treecomment[id(trees[0])]
            length = len(self._trees[0].leaves())
            width = pow(length, 1.5)*80 + 230 + pow(len(self._trees[0]), 2)*35
            self._cframe.add_widget(self.oldcomment.widget(), width, 0)
        else:
            if self.oldcomment:
                self._cframe.destroy_widget(self.oldcomment.widget())
                self.oldcomment = None

        self.show_fs = show
        for i in self._widgets:
            self._cframe.destroy_widget(i)
        self._size = IntVar(self._top)
        self._size.set(12)
        bold = ('helvetica', -self._size.get(), 'bold')
        helv = ('helvetica', -self._size.get())


        self._width = int(ceil(sqrt(len(trees))))
        self._widgets = []
        for i in range(len(trees)):
            if isinstance(trees[i], TAGTree):
                fs = trees[i].get_all_fs()
            else:
                fs = None
            widget = TAGTreeWidget(self._cframe.canvas(), trees[i], fs,
                                show, self._top, leaf_color='#008040',
                                roof_color='#004040', roof_fill='white',
                                line_color='#004040', leaf_font=helv, 
                                **attribs)
            widget['yspace'] = 70
            widget['xspace'] = 70
            self._widgets.append(widget)
            self._cframe.add_widget(widget, 0, 0)

        
        self._layout()
        self._cframe.pack(expand=1, fill='both', side = LEFT)

    def pack(self, cnf={}, **kw):
        """
        Pack the canvas frame of ``TreeView``.  See the documentation 
        for ``Tkinter.Pack`` for more information.
        """
        self._cframe.pack(cnf, **kw)

class TAGTreeSetView(object):
    """
    A window that displays a TAG Tree set. TAG Tree Set
    contains a group of TAG trees, when clicking the tree
    name on the tree list, the tree will be displayed on
    the canvas.

    """
    def __init__(self, tagtrees, parent=None):
        self._trees = tagtrees

        if parent is None:
            self._top = Tk()
            self._top.title('NLTK')
            self._top.bind('<Control-p>', lambda e: self.print_to_file())
            self._top.bind('<Control-x>', self.destroy)
            self._top.bind('<Control-q>', self.destroy)
            self._top.geometry("1400x800")
        else:
            self._top = parent[0]

        frame = Frame(self._top)

        v = StringVar()
        self.w = Label(frame, text='Regexp:')
        self._e = Entry(frame, textvariable=v)
        self._e.bind("<Return>", self.return_pressed)
        self._show_fs = True
        self._show_fs_button = Button(frame, text="Hide Features",
                                      command=self.show_fs)
        self._sfs_button = Button(frame, text="Add Start Features",
                                  command=self.start_feat)
        self.highlight_button = Button(frame, text="Highlight",
                                       command=self.highlight)
        self.remove_button = Button(frame, text="Remove",
                                    command=self.remove)
        self.keep_button = Button(frame, text="Keep",
                                  command=self.keep)
        self._sfs_button.pack(side=LEFT)
        self._show_fs_button.pack(side=LEFT)
        self.w.pack(side=LEFT)
        self._e.pack(expand=1, fill='both', side = LEFT)
        self.highlight_button.pack(side=RIGHT)
        self.keep_button.pack(side=RIGHT)
        self.remove_button.pack(side=RIGHT)
        

        statframe = Frame(self._top)
        self.notfl = Label(statframe, text='Tree Framilies:')
        self.notf = StringVar()
        self.nott = StringVar()
        self.tfcl = Label(statframe, textvariable=self.notf)
        self.nottl = Label(statframe, text='Trees:')
        self.tcl = Label(statframe, textvariable=self.nott)

        self.notf.set(str(self._trees.tree_family_count()))
        self.nott.set(str(self._trees.tree_count()))

        statframe.pack(side = BOTTOM, fill='both')
        self.notfl.pack(side = LEFT)
        self.tfcl.pack(side = LEFT)
        self.nottl.pack(side = LEFT)
        self.tcl.pack(side = LEFT)

        frame.pack(side = BOTTOM, fill='both')

        self._frame = Frame(self._top)
        self._frame.pack(fill='both', side = LEFT)
        
        self.cols = ('fullpath', 'type') 
        self._add_fs = True
        self._tagview = ttk.Treeview(self._frame, columns=self.cols,
                            displaycolumns='',
                            yscrollcommand=lambda f, l:autoscroll(vsb, f, l),
                            xscrollcommand=lambda f, l:autoscroll(hsb, f, l))
        ysb = ttk.Scrollbar(self._frame, orient=VERTICAL, 
                            command=self._tagview.yview)
        xsb = ttk.Scrollbar(self._frame, orient=HORIZONTAL,
                            command=self._tagview.xview)
        self._tagview['yscroll'] = ysb.set
        self._tagview['xscroll'] = xsb.set
        if parent:
            self._tagview.bind('<<TreeviewSelect>>', parent[1])
        else:
            self._tagview.bind('<<TreeviewSelect>>', self.display)
        self.populate_tree('', self._trees)
        self._tagview.configure(xscrollcommand=xsb.set,
                                yscrollcommand=ysb.set)
        ysb.pack(fill='y', side='right')
        xsb.pack(fill='x', side='bottom')

        self._tagview.heading('#0', text='Trees', anchor=W)
        self._tagview.column('#0', stretch=1, width=220)

        self._tagview.pack(expand=1, fill='both')
        self._tw = TAGTreeView(None, parent=self._top)
        self._tw.pack(expand=1, fill='both', side = LEFT)
        self.sfs_tree ={}

    def return_pressed(self, event):
        """
        Short-cut for pressing return to show feature structures
        """
        words = self._e.get().split()
        if len(words) == 0:
            self._show_fs = False
            self.show_fs()
        return

    def start_feat(self):
        """
        Add or Remove start feature structure of TAG tree set
        """
        if self._trees.start_fs is None:
            raise TypeError("Should set start feature for TAG Trees First")
        node = self._tagview.focus()
        path = self._tagview.set(node, "fullpath").split('/')
        tree = self._trees
        for subpath in path[1:]:
            if subpath in tree:
                tree = tree[subpath]
            else:
                raise TypeError("%s: tree does not match"
                             % type(self).__name__)
        if isinstance(tree, type(self._trees)):
            return
        if not tree.start_feat:
            self._sfs_button['text'] = 'Delete Start Features'
            self.add_start_fs(tree, self._trees.start_fs)
            tree.start_feat = True
        else:
            self._sfs_button['text'] = 'Add Start Features'
            self.del_start_fs(tree)
            tree.start_feat = False
        self._add_fs = not self._add_fs

    def add_start_fs(self, tree, start_fs):
        """
        Add start feature structure.
        :param tree: display tree
        :type: TAGTree
        :param start_fs: start feature structure
        :type: FeatStruct
        """
        root = tree.get_node_name() + '.t'
        all_fs = tree.get_all_fs()
        self._old_sfs = copy.deepcopy(all_fs[root])
        for i in start_fs:
            all_fs[root][i] = start_fs[i]
        tree.set_all_fs(all_fs)
        self._tw.redraw(self._show_fs, tree)

    def del_start_fs(self, tree):
        """
        Remove start feature structure.
        :param tree: display tree
        :type: TAGTree
        """
        root = tree.get_node_name() + '.t'
        all_fs = tree.get_all_fs()
        all_fs[root] = self._old_sfs
        tree.set_all_fs(all_fs)
        self._old_sfs = None
        self._tw.redraw(self._show_fs, tree)
    
    def pack(self):
        """
        Pack the canvas frame of ``TAGTreeView``.
        """
        self._tagview.pack(expand=1, fill='both')
        self._frame.pack(fill='both', side = LEFT)
        self._tw.pack(expand=1, fill='both', side = LEFT)

    def focus(self):
        """
        Get selected TAGTree
        :return: selected treeview
        :rtype: TAGTree
        """
        node = self._tagview.focus()
        path = self._tagview.set(node, "fullpath").split('/')
        tree = self._trees
        for subpath in path[1:]:
            if subpath in tree:
                if not isinstance(tree, type(self._trees)):
                    if tree._lex:
                        tree[subpath] = tree[subpath].copy(True)

                tree = tree[subpath]
            else:
                raise TypeError("%s: tree does not match"
                             % type(self).__name__)

        if not isinstance(tree, type(self._trees)):
            if tree._lex:
                tree.lexicalize()
                tree._lex = False
            return (tree, subpath)

    def display(self, event=None):
        """
        Display the tag tree on the canvas when the tree
        is selected.
        """
        node = self._tagview.focus()
        path = self._tagview.set(node, "fullpath").split('/')
        tree = self._trees
        for subpath in path[1:]:
            if subpath in tree:
                if not isinstance(tree, type(self._trees)):
                    if tree._lex:
                        tree[subpath] = tree[subpath].copy(True)

                tree = tree[subpath]
            else:
                raise TypeError("%s: tree does not match"
                             % type(self).__name__)

        if not isinstance(tree, type(self._trees)):
            if tree.start_feat:
                self._sfs_button['text'] = 'Delete Start Features'
            else:
                self._sfs_button['text'] = 'Add Start Features'
            if tree._lex:
                tree.lexicalize()
                tree._lex = False
                self._tw.redraw(self._show_fs, tree)
            else:
                self._tw.redraw(self._show_fs, tree)

    def populate_tree(self, parent, trees):
        """
        Popluate the trees on the treeview.
        """
        if not trees:
            return
        for t in sorted(trees.keys()):
            node = parent
            parent_path = self._tagview.set(parent, "fullpath")
            path = parent_path + '/' + t
            if ord(t[0]) < 10:
                f_chr = self.greek(t[0])
            else:
                f_chr = t[0]
            if isinstance(trees[t], type(trees)):
                node = self._tagview.insert(parent, END, text=f_chr+t[1:],
                                      values=[path, 'directory'])
                self.populate_tree(node, trees[t])
            else:
                self._tagview.insert(parent, END, text=f_chr+t[1:],
                                      values=[path, 'file'])
    
    def clear(self):
        """
        Empty the treeview and TAG tree set.
        """
        self._sfs_button['text'] = 'Add Start Features'
        self._add_fs = True
        x = self._tagview.get_children()
        for item in x: 
            self._tagview.delete(item)
        self._trees = TAGTreeSet()

    def update(self, trees):
        """
        Update the window when the change the TAG tree set.
        """
        self._tw.clear()
        self._trees = trees
        self.populate_tree('', trees)
        self.notf.set(str(self._trees.tree_family_count()))
        self.nott.set(str(self._trees.tree_count()))

    def greek(self, ascii):
        """
        Translate ASCII to greek letter
        """    
        i = ord(u'\u03af') + ord(ascii)
        return chr(i)

    def destroy(self, *e):
        if self._top is None: return
        self._top.destroy()
        self._top = None

    def mainloop(self, *args, **kwargs):
        """
        Enter the Tkinter mainloop.  This function must be called if
        this demo is created from a non-interactive program (e.g.
        from a secript); otherwise, the demo will close as soon as
        the script completes.
        """
        if in_idle(): return
        self._top.mainloop(*args, **kwargs)

    def show_fs(self):
        """
        Display or hide the feature structure on the canvas.
        """
        if self._show_fs:
            self._show_fs_button['text'] = 'Show Feature'
        else:
            self._show_fs_button['text'] = 'Hide Feature'
        self._e.delete(0, END)
        self._show_fs = not self._show_fs
        node = self._tagview.focus()
        path = self._tagview.set(node, "fullpath").split('/')
        tree = self._trees
        for subpath in path[1:]:
            if subpath in tree:
                tree = tree[subpath]
            else:
                raise TypeError("%s: tree does not match"
                             % type(self).__name__)
        if not isinstance(tree, type(self._trees)):
            self._tw.redraw(self._show_fs, tree)

    def keep(self):
        """
        Display the feature structures which match the
        input regular expression.
        """
        node = self._tagview.focus()
        path = self._tagview.set(node, "fullpath").split('/')
        tree = self._trees
        for subpath in path[1:]:
            if subpath in tree:
                tree = tree[subpath]
            else:
                raise TypeError("%s: tree does not match"
                             % type(self).__name__)
        if not isinstance(tree, type(self._trees)):
            self._tw.redraw(self._show_fs, tree, keep=True, reg=self._e.get())
        self._show_fs_button['text'] = 'Show ALL Features'
        self._show_fs = False
        #return

    def highlight(self):
        """
        Remove the feature structures which match the
        input regular expression from the canvas.
        """
        node = self._tagview.focus()
        path = self._tagview.set(node, "fullpath").split('/')
        tree = self._trees
        for subpath in path[1:]:
            if subpath in tree:
                tree = tree[subpath]
            else:
                raise TypeError("%s: tree does not match"
                             % type(self).__name__)
        if not isinstance(tree, type(self._trees)):
            self._tw.redraw(self._show_fs, tree, 
                            highlight=True, reg=self._e.get())

    def remove(self):
        """
        Highlight the feature structures which match 
        the input regular expression.
        """
        node = self._tagview.focus()
        path = self._tagview.set(node, "fullpath").split('/')
        tree = self._trees
        for subpath in path[1:]:
            if subpath in tree:
                tree = tree[subpath]
            else:
                raise TypeError("%s: tree does not match"
                             % type(self).__name__)
        if not isinstance(tree, type(self._trees)):
            self._tw.redraw(self._show_fs, tree, 
                            remove=True, reg=self._e.get())
        self._show_fs_button['text'] = 'Show ALL Features'
        self._show_fs = False

class CommentWidget(object):
    """
    A canvas widget that displays a comment of a tree.
    Click to collapse or close the comment

    Attributes:

      - ``xposition``: the x cordinates of the widget on canvas
      - ``yposition``: the y cordinates of the widget on canvas
      - ``width``: width of comment width

    """
    def __init__(self, canvas, parent, **attribs):
        self._attr = {}
        next_attribs = {}
        for (i, j) in attribs.items():
            if i in ['xposition', 'yposition', 'width']:
                self._attr[i] = j
            else:
                next_attribs[i] = j
        attribs = next_attribs
        self.canvas = canvas
        self.attribs = attribs
        self.viewer = parent
        self.tri = TextWidget(canvas, '\u25bc' + '    COMMENTS')
        self.tri.bind_click(self.collapse)
        comment = TextWidget(self.canvas, self.viewer._trees[0].comment, 
                                  width=self._attr['width']-40)
        hspace1 = SpaceWidget(self.canvas, self._attr['width'], 0)
        hspace2 = SpaceWidget(self.canvas, self._attr['width'], 0)              
        tstack = StackWidget(self.canvas, hspace1, comment, hspace2)
        box = BoxWidget(self.canvas, tstack)
        self.stack = StackWidget(self.canvas, self.tri, box, align='left')
        self.show = True

    def widget(self):
        return self.stack

    def collapse(self, event):
        if self.show:
            self.viewer._cframe.destroy_widget(self.stack)
            self.tri = TextWidget(self.canvas, 
                                  '\u25b6' + '    COMMENTS')
            self.tri.bind_click(self.collapse)
            self.stack = StackWidget(self.canvas, self.tri, **self.attribs)
        else:
            self.viewer._cframe.destroy_widget(self.stack)
            self.tri = TextWidget(self.canvas, 
                                  '\u25bc' + '    COMMENTS')
            self.tri.bind_click(self.collapse)
            comment = TextWidget(self.canvas, self.viewer._trees[0].comment, 
                                  width=self._attr['width']-40)
            hspace1 = SpaceWidget(self.canvas, self._attr['width'], 0)
            hspace2 = SpaceWidget(self.canvas, self._attr['width'], 0)              
            tstack = StackWidget(self.canvas, hspace1, comment, hspace2)
            box = BoxWidget(self.canvas, tstack)
            self.stack = StackWidget(self.canvas, self.tri, box, align='left')
        self.show = not self.show
        self.viewer.collapse_comment()

class TAGTreeWidget(TreeWidget):
    """
    A canvas widget that displays a single TAG Tree, inherit from 
    ``TreeWidget``. ``TAGTreeWidget`` manages a group of ``TreeSegmentWidgets``
    that are used to display a TAG Tree. The each TAG Tree node
    contains a top feature structure and a bottom feature structure.
    The feature structures can be set to display or hidden on the canvas.

    Attributes:

      - ``keep``: Display the feature structures which match the
        input regular expression.
        node widgets for this ``TreeWidget``.
      - ``remove``: Remove the feature structures which match the
        input regular expression from the canvas.
      - ``highlight``: Highlight the feature structures which match 
        the input regular expression.
      - ``reg``: The input regular expression for keep, remove or
        highlight.

    """
    def __init__(self, canvas, t, fs, show, parent, make_node=TextWidget,
                 make_leaf=TextWidget, **attribs):
        self._all_fs = fs
        self._show_fs = show
        self._attr = {}

        for (i, j) in attribs.items():
            if i in ['keep', 'remove', 'highlight', 'reg']:
                self._attr[i] = j
                del attribs[i]
        TreeWidget.__init__(self, canvas, t, make_node,
                            make_leaf, **attribs)   
        self._top = parent

    def _make_expanded_tree(self, canvas, t, key):        
        make_node = self._make_node
        make_leaf = self._make_leaf

        if isinstance(t, TAGTree):
            node_name = self.get_node_name(t)
            bold = ('helvetica', -24, 'bold')
            node = make_node(canvas, node_name, font=bold,
                                color='#004080')

            if self._show_fs:
                top_fs = feat_widget(t.top_fs, self._all_fs, canvas, 
                               global_featname(node_name, True), **self._attr)
                bot_fs = feat_widget(t.bot_fs, self._all_fs, canvas,
                            global_featname(node_name, False), **self._attr)
                cstack = StackWidget(canvas, top_fs, bot_fs, align='left')
                node =  SequenceWidget(canvas, node, cstack, align='top')

            self._nodes.append(node)
            children = t
            subtrees = [self._make_expanded_tree(canvas, children[i], key+(i,))
                        for i in range(len(children))]
            try:
                label = t.node
            except NotImplementedError:
                label = t._label
            top_name = global_featname(label, True)
            bot_name = global_featname(label, False)
            top_fs = FeatStruct()
            bot_fs = FeatStruct()
            if top_name in self._all_fs:
                top_fs = self._all_fs[top_name]
            if bot_name in self._all_fs:
                bot_fs = self._all_fs[bot_name]

            treeseg = TreeSegmentWidget(canvas, node, subtrees, 
                                        color=self._line_color,
                                        width=self._line_width)
            self._expanded_trees[key] = treeseg
            self._keys[treeseg] = key

            return treeseg
        else:
            leaf = make_leaf(canvas, t, **self._leafattribs)
            self._leaves.append(leaf)
            return leaf

    def get_node_name(self, t):    
        """
        Get the name of the current node, use specific symbols
        for substitution node, head node and foot node.
        :param t: Current node
        :type t: TAGTree
        :return: Node name
        :rtype: str
        """   
        try:
            label = t.node
        except NotImplementedError:
            label = t._label 
        if t.attr == 'subst':
            return label + '\u2193'
        elif t.attr == 'head':
            return label + '\u25c7'
        elif t.attr == 'foot':
            return label + '\u2605'
        elif t.attr == None:
            return label
        elif t.attr == 'lex':
            return label
        else:
            raise TypeError("%s: Expected an attribute with value"
                            "subst, head or foot ")

def feat_widget(fs, allfs, canvas, name, **attribs):
    """
    Get the widget of feature structure.

    param: fs, the current feature structure to display
    param: allfs, the overall feature structure of the
    tree
    param: canvas, the canvas the feature structure 
    widget to attach
    param: name, the node name of the feature structure
    param: attribs, the attribs of the widget

    Attributes:

      - ``keep``: Display the feature structures which match the
        input regular expression.
        node widgets for this ``TreeWidget``.
      - ``remove``: Remove the feature structures which match the
        input regular expression from the canvas.
      - ``highlight``: Highlight the feature structures which match 
        the input regular expression.
      - ``reg``: The input regular expression for keep, remove or
        highlight.
    """
    if fs:
        return feat_to_widget(allfs, canvas, name, **attribs)
    else:
        return TextWidget(canvas, '[ ]', justify='center')

def global_featname(node_name, top):
    try:
        node_name[-1]
    except UnicodeDecodeError:
        node_name = node_name[:-3]
    if top:
        return '.'.join([node_name, 't'])
    else:
        return '.'.join([node_name, 'b'])


def feat_to_widget(f, c, name, **attribs):
    """
    Draw a widget of the feature structure
    :param f: feature struct
    :type: FeatStruct
    :param c: parent canvas
    :type: CanvasFrame

    Attributes:

      - ``keep``: Display the feature structures which match the
        input regular expression.
        node widgets for this ``TreeWidget``.
      - ``remove``: Remove the feature structures which match the
        input regular expression from the canvas.
      - ``highlight``: Highlight the feature structures which match 
        the input regular expression.
      - ``reg``: The input regular expression for keep, remove or
        highlight.
    """
    feats = copy.deepcopy(f)
    old_name = []
    new_feats = {}
    for fname in feats:
        if ord(fname[0]) > 206:
            f_chr = ord('\u03af') + ord(fname[0])
            uname = str(f_chr) + fname[1:]
            new_feats[uname] = feats[fname]
            old_name.append(fname)
        else:
            new_feats[fname] = feats
    feats = new_feats
    for name in old_name:
        if name in feats:
            del feats[name]

    remove_or_tag(feats)

    fstr = feats.__repr__()
    fstr = fstr.replace('__value__=','')
    fstr = fstr.replace("'",'')
    # HACK BECAUSE EVERYTHING IS BROKEN BELOW
    return TextWidget(c, '[ ]', justify='center')
    
    d = {}
    for (attr, value) in attribs.items(): 
        d[attr] = value
    reg = None
    if 'reg' in d:
        reg = d['reg']
    s = fstr.find(name)
    l = find_left_bracket(fstr[s:])
    r = match_bracket(fstr[s:])
    match_str = fstr[s+l+1:s+r]
    result = match_str

    if s+l+1 == s+r or not match_str:
        return TextWidget(c, '[ ]', justify='center')

    if reg and len(reg) > 0:
        if 'highlight' in d:
            match_list = re.split('(\[|\,|\])', match_str)
            result = ''
            reg = reg.replace(',','')
            reg = reg.replace(']','')
            for i in match_list:
                try:    
                    result += re.compile(r'((%s\s*)+)' %
                              reg).sub(r'<h>\1<h>', i)
                except [re.error, e]:
                    widget = str_to_widget(match_str, c)
                    return BracketWidget(c, widget, color='black', width=1)
            widget = str_to_widget(result, c, True)
        elif 'keep' in d:
            fstr = match_feature(feats, reg, 0).__repr__()
            fstr = fstr.replace("'",'')
            fstr = fstr.replace('__value__=','')
            s = fstr.find(name)
            l = find_left_bracket(fstr[s:])
            r = match_bracket(fstr[s:])
            if l and r:
                result = fstr[s+l+1:s+r]
            else:
                return TextWidget(c, '[ ]', justify='center')
            widget = str_to_widget(result, c)
        elif 'remove' in d:
            fstr = match_feature(feats, reg, 1).__repr__()
            fstr = fstr.replace('__value__=','')
            fstr = fstr.replace("'",'')
            s = fstr.find(name)
            l = find_left_bracket(fstr[s:])
            r = match_bracket(fstr[s:])
            if l and r:
                result = fstr[s+l+1:s+r]
            else:
                result = match_str
            widget = str_to_widget(result, c)
    else:
        widget = str_to_widget(result, c)
    return BracketWidget(c, widget, color='black', width=1)

def str_to_widget(fstr, c, highlight=False):
    """
    Parse the string of feature structure into a canvas widget
    :param fstr: feat structure representation string
    :type: string
    :param c: parent canvas
    :type: CanvasFrame
    :param highlight: whether to highlight key words
    :type: bool
    """
    if fstr[:4] == '<h>,':
        fstr = fstr[4:]
    if fstr[-4:] == ']<h>':
        fstr = fstr[:-3]
    fstr = fstr.replace("<h><h>", "<h>")
    fstr = fstr.replace("<h>><h>", ">")
    fstr = fstr.replace("<h>-<h>>", "->")
    wl = []
    l = find_left_bracket(fstr)
    if l:
        r = match_bracket(fstr)
        lw = str_to_widget(fstr[l+1:r], c, highlight)
        tl = []
        strlist = fstr[:l].split(',')
        for item in strlist[:-1]:
            if len(item) > 0:
                if highlight and ('<h>' in item):
                    tl.append(BoxWidget(c, TextWidget(c, '%s' %
                              item.replace('<h>',''), justify='center'),
                              outline='white', fill='yellow'))
                else:
                    tl.append(TextWidget(c, '%s' % item, justify='center'))
        if highlight and ('<h>' in strlist[-1]):
            tw = BoxWidget(c, TextWidget(c, '%s' % 
                           strlist[-1].replace('<h>',''), justify='center'),
                           outline='white', fill='yellow')
        else:
            tw = TextWidget(c, '%s' % strlist[-1], justify='center')
        if not isinstance(lw, TextWidget):
            lw = BracketWidget(c, lw, color='black', width=1)
        tl.append(SequenceWidget(c, tw, lw, align='center'))
        wl.append(StackWidget(c, *tl))
        if r+1 != len(fstr):
            wl.append(str_to_widget(fstr[r+1:len(fstr)], c, highlight))
    else:
        tl = []
        textl = fstr.split(',')
        textl = filter(None, textl)
        if len(textl) == 0:
            return TextWidget(c, '[ ]', justify='center')
        elif highlight:
            for item in textl:
                item = item.replace('__value__=','')
                item = item.replace("'",'')
                if '<h>' in item:
                    tl.append(BoxWidget(c, TextWidget(c, '%s' % 
                            item.replace('<h>',''), justify='center'),
                            outline='white', fill='yellow'))
                else:
                    tl.append(TextWidget(c, '%s' % item, justify='center'))
            return StackWidget(c, *tl)
        elif len(textl) == 1:
            if '->(' in textl[0]:
                return TextWidget(c, '%s' % textl[0], justify='center')
            else:
                return TextWidget(c, '[ %s ]' % textl[0], justify='center')
        else:
            for item in textl:
                tl.append(TextWidget(c, '%s' % item, justify='center'))
            return StackWidget(c, *tl)
    cstack = StackWidget(c, *wl)
    return cstack

def remove_or_tag(feat):
    """
    Remove "__or_" key in feature structure when display
    :type: FeatStruct
    """
    if isinstance(feat, str):
        return
    keys = feat.keys()
    length = len(keys)
    for key in list(keys):
        try:
            key.decode('ascii')
        except:
            if len(key) >= 4:
                value = feat[key]
                feat['eps'+key[2:]] = value
                del feat[key]
                key = 'eps'+key[2:]
        if key[:5] == '__or_' and length > 1:
            values = feat.values()
            value = ''
            for v in values:
                value = value + v + '/'
            feat['__value__'] = value[:-1]
            for key in keys:
                del feat[key]
            return
        elif key[:5] == '__or_':
            value = feat[key]
            feat['__value__'] = value
            del feat[key]
        elif key == '__value__':
            kvalue = feat[key]
        else:
            remove_or_tag(feat[key])

def demo():
    g = Grammar('english')
    viewer = LexView(g)
    viewer.mainloop()

if __name__ == '__main__':
    demo()
