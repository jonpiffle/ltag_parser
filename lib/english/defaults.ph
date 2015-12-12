#!/usr/bin/perl -w

# family_defaults:
# tree_defaults:

# when entries in the morph are not found in the syn, these default
# family and tree entries are assigned to those entries.

%family_defaults = 
    (
#     "Pron" => "Tnx0N1 Ts0N1",
#     "PropN" => "Tnx0N1 Ts0N1",
     "V" => "Tnx0Vnx1 Tnx0V",
     "A" => "Tnx0Ax1",
     "P" => "Tnx0Pnx1"
     );

%tree_defaults = 
    (
     "Pron" => "N NXN",
     "PropN" => "N Nn NXN",
     "N" => "N Nn NXN",
     "D" => "Dnx D",
     "G" => "nxGnx NXnxG DnxG",
     "P" => "P PXPnx nxPnx vxPnx spuPnx Pnxs Pss",
     "A" => "An A AXA",
     "I" => "Is",
     "Ad" => "Ad ARBvx vxARB spuARB ARBs sARB arbARB ARBarb ARBpx pxARB ARBa",
     "Conj" => "nx1CONJnx2 a1CONJa2 px1CONJpx2 arb1CONJarb2 s1CONJs2 CONJs",
     "Comp" => "COMPs"
     );

# feat_defaults: also assign some morph features by default

%feat_defaults = 
    (
     "PropN" => "WH- refl- decreas- definite+ GEN- quan-",
     "N" => "WH- refl- decreas- GEN- quan-",
     "Pron" => "WH- refl- pron+ card- const- decreas- definite+ GEN- quan-",
     "Det" => "WH-",
     "A" => "WH-",
     "Prep" => "p_acc"
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
     "WH-" => "WH WH+",
     "GEN-" => "GEN+"
     );

# feat_disjuncts:

# for a morph entry which has a certain underspecified feature, this
# hash table is used to expand this feature into several disjunctive
# features

%feat_disjuncts = 
    (
     "INF" => "IND BASE SBJNCT"
     );


