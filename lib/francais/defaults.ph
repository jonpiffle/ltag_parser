#!/usr/bin/perl -w

# family_defaults:
# tree_defaults:

# when entries in the morph are not found in the syn, these default
# family and tree entries are assigned to those entries.

%family_defaults = 
    (
     "V" => "n0V-a n0Vn1",
     "Ad" => "VAd",
     "Prep" => "VSP"
     );

%tree_defaults = 
    (
     "N" => "N",
     "D" => "DN",
     "Prep" => "NPn1",
     "A" => "aN A Na",
     "I" => "SI",
     "Ad" => "adA",
     "Conj" => "Conj nCn sCs aCa adCad pCp",
     "Cl" => "Cl"
     );

# feat_defaults: also assign some morph features by default

%feat_defaults = 
    (
#     "PropN" => "WH- refl- decreas- definite+ GEN- quan-",
#     "N" => "WH- refl- decreas- GEN- quan-",
#     "Pron" => "WH- refl- pron+ card- const- decreas- definite+ GEN- quan-",
#     "Det" => "WH-",
#     "A" => "WH-",
#     "Prep" => "p_acc"
     );

# feat_conflicts: 

# when assigning a feature by default (e.g. WH-) using feat_defaults
# (defined above) check to see if certain features can conflict with
# the default assignment. If they do, then do not assign the default
# feature structure.  In the following, both WH and WH+ expand to
# <wh>=+ which conflicts with the default WH- which expands to <wh>=-;
# both of these features structures cannot be assigned to a single
# entry

%feat_conflicts = 
    (
#     "WH-" => "WH WH+",
#     "GEN-" => "GEN+"
     );

# feat_disjuncts:

# for a morph entry which has a certain underspecified feature, this
# hash table is used to expand this feature into several disjunctive
# features

%feat_disjuncts = 
    (
#     "INF" => "IND BASE SBJNCT"
     );

1;

