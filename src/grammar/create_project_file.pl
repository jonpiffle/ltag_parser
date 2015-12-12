# -*- perl -*-

my $rcsid = '$Id: create_project_file.pl,v 1.2 2000/11/05 05:06:48 anoop Exp $';

sub copy_epsilons_file
{
    open(EPSIN, "$EPSILON_IN") or die "could not open $EPSILON_IN";
    open(EPS, ">$EPSILONS") or die "could not open $EPSILONS";
    $num_eps = 0;
    while (<EPSIN>)
    {
	$num_eps++;
	print EPS $_;
    }
    close(EPSIN);
    close(EPS);
    print STDERR "copied ($num_eps) empty elements from $EPSILON_IN\n";
}

sub copy_toplabels_file
{
    open(TIN, "$TOPLABELS_IN") or die "could not open $TOPLABELS_IN";
    open(T, ">$TOPLABELS") or die "could not open $TOPLABELS";
    $num_top = 0;
    while (<TIN>)
    {
	$num_top++;
	print T $_;
    }
    close(TIN);
    close(T);
    print STDERR "copied ($num_top) top labels from $TOPLABELS_IN\n";
}

sub copy_defaults_file
{
    open(DEFIN, "$DEFAULTS_IN") or die "could not open $DEFAULTS_IN";
    open(D, ">$DEFAULTS") or die "could not open $DEFAULTS";

    while (<DEFIN>) {
	print D $_;
    }

    close(DEFIN);
    close(D);
    print STDERR "copied syntax defaults from $DEFAULTS_IN\n";
}

sub create_project_file
{
    open(PRJ, ">$PROJECTFILE") or die "could not open $PROJECTFILE";
    print STDERR "creating project file $PROJECTFILE\n";
    print PRJ "$numTrees_tag=$num_trees\n";
    print PRJ "$numFamilies_tag=$num_families\n";
    print PRJ "$numTreenodes_tag=$num_treenodes\n";
    print PRJ "$numLabels_tag=$num_labels\n";
    print PRJ "$numSubscripts_tag=$num_subscripts\n";
    print PRJ "$widthLabels_tag=$maxwidth_labels\n";
    print PRJ "$widthSubscripts_tag=$maxwidth_subscripts\n";
    print PRJ "$elementaryTrees_tag=$ELEM_TREES\n";
    print PRJ "$treeNames_tag=$TREE_NAMES\n";
    print PRJ "$treeNodes_tag=$TNODES\n";
    print PRJ "$treeFeatures_tag=$TREE_FEATS\n";
    print PRJ "$treeFamilies_tag=$TREE_FAMS\n";
    print PRJ "$treeLabels_tag=$LABELS\n";
    print PRJ "$treeSubscripts_tag=$SUBSCRIPTS\n";
    print PRJ "$wordDB_tag=$SYNFILE\n";
    print PRJ "$numepsilons_tag=$num_eps\n";
    print PRJ "$epsilons_tag=$EPSILONS\n";
    print PRJ "$numtoplabels_tag=$num_top\n";
    print PRJ "$toplabels_tag=$TOPLABELS\n";
    print PRJ "$defaults_tag=$DEFAULTS\n";
    print PRJ "$modelfile_tag=$MODELFILE\n" if ($MODELFILE ne "");
    close(PRJ);

    open(PRJ, ">$INPROJECTFILE") or die "could not open $INPROJECTFILE";
    print STDERR "creating project file $INPROJECTFILE\n";
    print PRJ "$numTrees_tag=$num_trees\n";
    print PRJ "$numFamilies_tag=$num_families\n";
    print PRJ "$numTreenodes_tag=$num_treenodes\n";
    print PRJ "$numLabels_tag=$num_labels\n";
    print PRJ "$numSubscripts_tag=$num_subscripts\n";
    print PRJ "$widthLabels_tag=$maxwidth_labels\n";
    print PRJ "$widthSubscripts_tag=$maxwidth_subscripts\n";
    print PRJ "$elementaryTrees_tag=$PRJ_ELEM_TREES\n";
    print PRJ "$treeNames_tag=$PRJ_TREE_NAMES\n";
    print PRJ "$treeNodes_tag=$PRJ_TNODES\n";
    print PRJ "$treeFeatures_tag=$PRJ_TREE_FEATS\n";
    print PRJ "$treeFamilies_tag=$PRJ_TREE_FAMS\n";
    print PRJ "$treeLabels_tag=$PRJ_LABELS\n";
    print PRJ "$treeSubscripts_tag=$PRJ_SUBSCRIPTS\n";
    print PRJ "$wordDB_tag=$PRJ_SYNFILE\n";
    print PRJ "$numepsilons_tag=$num_eps\n";
    print PRJ "$epsilons_tag=$PRJ_EPSILONS\n";
    print PRJ "$numtoplabels_tag=$num_top\n";
    print PRJ "$toplabels_tag=$PRJ_TOPLABELS\n";
    print PRJ "$defaults_tag=$PRJ_DEFAULTS\n";
    print PRJ "$modelfile_tag=$MODELFILE\n" if ($MODELFILE ne "");
    close(PRJ);
}

1;

