# -*- perl -*-
#--------------------------------------------------
# DEFINES
#--------------------------------------------------

package Tree;

if (!defined &_tree_ph) {
    eval 'sub _tree_ph {1;}';
    $TRUE = 1;
    $FALSE = 0;
    $START_T = 0;
    $NULL_T = -1;
    $ALPHA_S = "alpha";
    $BETA_S = "beta";
    $EPSILON_S = "";
}

