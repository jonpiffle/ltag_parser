#!/usr/bin/perl -w

%label_to_standard_pos_conversion = 
    (
#     "A" => "A",
#     "Ad" => "Ad",
#     "Comp" => "Comp",
#     "Conj" => "Conj",
#     "Conj1" => "Conj",
#     "Conj2" => "Conj",
#     "D" => "D",
#     "D1" => "D",
#     "D2" => "D",
#     "G" => "G",
#     "I" => "I",
#     "N" => "N",
#     "N1" => "N",
#     "NP" => "NP",
#     "P" => "P",
#     "P1" => "P",
#     "P2" => "P",
#     "PL" => "PL",
#     "Punct" => "Punct",
#     "Punct1" => "Punct",
#     "Punct2" => "Punct",
#     "V" => "V",
     );

%morph_pos_to_standard_pos_conversion = 
    (
#     "A" => "A",
#     "Adv" => "Ad",
#     "Comp" => "Comp",
#     "Conj" => "Conj",
#     "Det" => "D",
#     "G" => "G",
#     "I" => "I",
#     "N" => "N",
#     "NVC" => "NVC",
#     "Part" => "PL",
#     "Prep" => "P",
#     "Pron" => "N",
#     "PropN" => "N",
#     "Punct" => "Punct",
#     "V" => "V",
#     "VVC" => "VVC",
     );

# "Neg" => ("Ad"), # Didn"t find "Neg" in the DB
# "PL" => ("PL"),  # Didn"t find "PL" in the DB
# "PropN" => ("G", "N"),	# if morph-entry has feature GEN->GEN else N
# Missing punctuation from *morph-cat-alist*

%feature_conditionals = ();
#    (
#     "GEN" => "G"
#     );

1;

