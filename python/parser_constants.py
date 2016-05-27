import functools

from nltk.corpus.util import LazyCorpusLoader
from nltk.corpus.reader import CategorizedBracketParseCorpusReader

from xtag.xtag_nltk.grammar import Grammar as NLTKGrammar
from xtag_verbnet.grammar import Grammar
from xtag_verbnet.verbnet import VerbNet, XTAGMapper
from xtag_verbnet.propbank import Propbank

class Constants(object):
    _instance = None
    def __new__(cls, *args, **kwargs):
        if not cls._instance:
            cls._instance = super(Constants, cls).__new__(
                                cls, *args, **kwargs)
        return cls._instance

    def __init__(self):
        self.parser_dir = "/Users/piffle/Development/ltag_parser"
        self.verbnet_tree_families = [
            "Tnx0Vnx2nx1",
            "Tnx0Vnx1s2",
            "Tnx0Vs1",
            "Tnx0V",
            "Tnx0Vnx1",
            "Tnx0Vpl",
            "Tnx0Vax1",
            "Tnx0Vplnx2nx1",
            "Tnx0Vpnx1",
            "Tnx0Vnx1pnx2",
            "TEnx1V",
            "Tnx0Vplnx1",
        ]

    @property
    @functools.lru_cache()
    def propbank(self):
        return Propbank.load()

    @property
    @functools.lru_cache(None)
    def vnet(cls):
        return VerbNet.load()

    @property
    @functools.lru_cache(None)
    def ptb(cls):
        return LazyCorpusLoader( # Penn Treebank v3: WSJ portions
                'ptb', CategorizedBracketParseCorpusReader, r'wsj/\d\d/wsj_\d\d\d\d.mrg',
                cat_file='allcats.txt', tagset='wsj'
            )

    @property
    @functools.lru_cache(None)
    def grammar(cls):
        return Grammar.load()

    @property
    @functools.lru_cache(None)
    def mapper(cls):
        return XTAGMapper.load()

    @property
    @functools.lru_cache(None)
    def syntax_dict(cls):
        return NLTKGrammar.load('english').syntax
