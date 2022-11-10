/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : taas_basics.c
 Author      : Matthias Thimm
 Version     : 2.0
 Copyright   : GPL3
 Description : Basic reasoning functions for AAFs for taas solvers.
 ============================================================================
 */

/**
 * Checks some easy cases for problems
 */
int taas__check_very_easy_cases(struct TaskSpecification *task, struct AAF* aaf){
	// for every semantics, any argument in a loop is neither credulously nor
	// skeptically accepted (except for skeptical reasoning with stable semantics
	// (if there is no stable extension))
	if((strcmp(task->problem,"DC") == 0 || strcmp(task->problem,"DS") == 0) && (strcmp(task->track,"DS-ST") != 0)){
		if(bitset__get(aaf->loops,task->arg))
			return COMPUTATION_ABORTED__ANSWER_NO;
	}
  // "aaf->initial" already contains all initial arguments
	// check some simple special cases
	if(bitset__next_set_bit(aaf->initial,0) == -1){
		// if there are no initial arguments, the grounded extension is empty;
		// furthermore the grounded extension is also a complete extension
		if(strcmp(task->track,"SE-GR") == 0 || strcmp(task->track,"SE-CO") == 0){
			return COMPUTATION_ABORTED__ANSWER_EMPTYSET;
		}
		if(strcmp(task->track, "EE-GR") == 0){
			return COMPUTATION_ABORTED__ANSWER_EMPTYEMPTYSET;
		}
		// no argument is accepted (cred.+skept) under grounded semantics
		// and skeptically accepted wrt. complete semantics
		if(strcmp(task->track, "DC-GR") == 0 || strcmp(task->track, "DS-GR") == 0 || strcmp(task->track, "DS-CO") == 0){
			return COMPUTATION_ABORTED__ANSWER_NO;
		}
    return COMPUTATION_FINISHED__EMPTY_GROUNDED;
	}else{
		// for every semantics, if an argument is initial
		// it is both credulously and skeptically accepted
		// (except for credulous reasoning with stable semantics
		// (if there is no stable extension))
		if((strcmp(task->problem,"DS") == 0 || strcmp(task->problem, "DC") == 0)
					&& (strcmp(task->track,"DC-ST") != 0)){
			if(bitset__get(aaf->initial,task->arg)){
				return COMPUTATION_ABORTED__ANSWER_YES;
			}
		}
	}
  return COMPUTATION_FINISHED__NONEMPTY_GROUNDED;
}

/**
 * Computes the grounded extension (only if return value == COMPUTATION_FINISHED)
 * if return value == COMPUTATION_ABORTED__ANSWER_YES: computing of grounded extension has been aborated
 *     as the answer of the query is already decided to be "YES" (only possible for DC/DS)
 * if return value == COMPUTATION_ABORTED__ANSWER_NO: computing of grounded extension has been aborated
 *     as the answer of the query is already decided to be "NO" (only possible for DC/DS)
 */
int taas__compute_grounded(struct TaskSpecification *task, struct AAF* aaf, struct Labeling* grounded){
	// all initial arguments are automatically in the grounded extension
  bitset__clone(aaf->initial, grounded->in);
	// all arguments attacked by initial arguments are out
	bitset__init(grounded->out, aaf->number_of_arguments);
  bitset__unsetAll(grounded->out);
	GSList* stack = (GSList*) malloc(sizeof(GSList*));
	stack = NULL;
	for(int a = bitset__next_set_bit(grounded->in,0); a != -1 ; a = bitset__next_set_bit(grounded->in, a+1)){
		int* sidx = (int*) malloc(sizeof(int));
		*sidx = a;
		stack = g_slist_prepend(stack,sidx);
		//printf("A: %s\n", aaf->ids2arguments[*sidx]);
	}
	while(g_slist_length(stack)>0){
		int* arg = (int*) stack->data;
		stack = stack->next;
		//printf("B: %s\n", aaf->ids2arguments[*arg]);
		for(GSList* node = aaf->children[*arg]; node != NULL; node = node->next){
			// if *(int*)node->data is already out, we can continue
			if(bitset__get(grounded->out,*(int*)node->data))
				continue;
			// check if we can already provide an answer for the problem
			if(*(int*)node->data == task->arg){
				// as *(int*)node->data is attacked by the grounded extension, it is neither
				// credulously nor skeptically accepted in any semantics
				// (except maybe for stable semantics and skeptical reasoning)
				if(strcmp(task->track,"DS-ST") != 0){
					return COMPUTATION_ABORTED__ANSWER_NO;
				}
			}
			//argument is out
			bitset__set(grounded->out,*(int*)node->data);
			//decrease attack counter by one for each child of *(int*)node->data
			for(GSList* node2 = aaf->children[*(int*)node->data]; node2 != NULL; node2 = node2->next){
				if(aaf->number_of_attackers[*(int*)node2->data] > 0){
					aaf->number_of_attackers[*(int*)node2->data]--;
					if(aaf->number_of_attackers[*(int*)node2->data] == 0){
						// check if we can already provide an answer for the problem
				    if(*(int*)node2->data == task->arg){
							// as *(int*)node2->data is in the grounded extension, it is both
							// credulously and skeptically accepted in any semantics
							// (except maybe for stable semantics and credulous reasoning)
							if(strcmp(task->track,"DC-ST") != 0){
								return COMPUTATION_ABORTED__ANSWER_YES;
							}
						}
						bitset__set(grounded->in,*(int*)node2->data);
						int* sidx = (int*) malloc(sizeof(int));
						*sidx = *(int*)node2->data;
						//printf("C: %s\n", aaf->ids2arguments[*sidx]);
						stack = g_slist_prepend(stack,sidx);
					}
				}
			}
		}
		free(arg);
	}
  // if we have not solved DS_GR, DC_GR, or DS_CO until now, the answer is definitely "NO" (otherwise we
  // would have found the argument to be in by now)
	if(strcmp(task->track,"DS-GR") == 0 || strcmp(task->track,"DC-GR") == 0 || strcmp(task->track, "DS-CO") == 0){
		return COMPUTATION_FINISHED__ANSWER_NO;
  }
  return COMPUTATION_FINISHED;
}

/**
 * The general solve method for taas solvers, handles easy cases
 * and does the input/output etc.
 */
int taas__solve(int argc,
				char *argv[],
				struct SolverInformation* info,
				void (*doSolve)(struct TaskSpecification*,struct AAF*, struct Labeling*)){
		// Handle command parameters
		struct TaskSpecification *task = taas__cmd_handle(argc,argv,info);
	  // if only solver information was asked, terminate
		if(task != NULL){
	    // read file
	    struct AAF *aaf = (struct AAF*) malloc(sizeof(struct AAF));
	    // NOTE: for ICCMA23, we only support i23
			//if(task->format != NULL && strcmp(task->format,"tgf") == 0)
			//	taas__readFile_tgf(task->file,aaf);
			//else
			taas__readFile_i23(task->file,aaf);
			// if DS or DC problem, parse argument under consideration
			taas__update_arg_param(task,aaf);
			// check for (very) easy cases
	    int ans = taas__check_very_easy_cases(task,aaf);
	    // this will hold the grounded extension if needed
	    struct Labeling* grounded;
	    char bool_grounded_init = 0;
			if(ans == COMPUTATION_FINISHED__NONEMPTY_GROUNDED){
	      // if the answer is not (very) easy and there are initial arguments
	      // compute now the complete grounded labeling
			  grounded = (struct Labeling*) malloc(sizeof(struct Labeling));
	      bool_grounded_init = 1;
	      taas__lab_init(grounded,FALSE);
	      ans = taas__compute_grounded(task,aaf,grounded);

	    }
	    // check if we already solved some easy cases
	    if(ans == COMPUTATION_ABORTED__ANSWER_YES){
	      printf("YES\n");
	    }else if(ans == COMPUTATION_ABORTED__ANSWER_NO || ans == COMPUTATION_FINISHED__ANSWER_NO){
				printf("NO\n");
	  	}else if(ans == COMPUTATION_ABORTED__ANSWER_EMPTYSET){
	  		printf("[]\n");
	    }else if(ans == COMPUTATION_ABORTED__ANSWER_EMPTYEMPTYSET){
	  		printf("[\n[]\n]\n");
	    }else if(strcmp(task->track,"SE-GR") == 0 || strcmp(task->track, "SE-CO") == 0 ){
	      printf("%s\n",taas__lab_print(grounded,aaf));
	    }else if(strcmp(task->track,"EE-GR") == 0){
	      printf("[\n%s\n]\n",taas__lab_print(grounded,aaf));
	    }else{
	      // at this point DS_GR, DC_GR, EE_GR, SE_GR, SE_CO, DS_CO are solved
				// if grounded has not been computed yet, it is empty
				if(!bool_grounded_init){
					grounded = (struct Labeling*) malloc(sizeof(struct Labeling));
		      bool_grounded_init = 1;
		      taas__lab_init(grounded,FALSE);
					bitset__init(grounded->in, aaf->number_of_arguments);
					bitset__unsetAll(grounded->in);
					bitset__init(grounded->out, aaf->number_of_arguments);
					bitset__unsetAll(grounded->out);
				}
	      doSolve(task,aaf,grounded);
	    }
	    if(bool_grounded_init) taas__lab_destroy(grounded);
	    // destroy aaf
	    taas__aaf_destroy(aaf);
	  }
	  taas__solverinformation_destroy(info);
	  taas__cmd_destroy(task);
		return 0;
}

/**
 * Checks wheter the argument "arg" is correctly labeled in
 * the given labeling wrt. the given AAF , i.e.
 * - if the argument is labeled "in" then all its attackers and all its
 *   attackees must be labeled "out"
 * - if the argument is labeled "out" then there must be at least one attacker
 *   labeled "in"
 * - if the argument is labeled "undec" then no attacker is labeled "in", there
 *   is at least one attacker labeled "undec", and no attackee is labeled "in"
 * This methods returns TRUE iff the argument is labeled correctly.
 */
int taas__labeled_correctly(struct AAF* aaf, struct Labeling* lab, int arg){
	int label = taas__lab_get_label(lab,arg);
	if(label == LAB_IN){
		// argument is "in"
		for(GSList* node = aaf->children[arg]; node != NULL; node = node->next)
			if(taas__lab_get_label(lab,*(int*)node->data) != LAB_OUT)
				return FALSE;
		for(GSList* node = aaf->parents[arg]; node != NULL; node = node->next)
			if(taas__lab_get_label(lab,*(int*)node->data) != LAB_OUT)
				return FALSE;
		return TRUE;
	}
	if(label == LAB_OUT){
		// argument is "out"
		for(GSList* node = aaf->parents[arg]; node != NULL; node = node->next)
			if(taas__lab_get_label(lab,*(int*)node->data) == LAB_IN)
				return TRUE;
		return FALSE;
	}
	// argument is "undec"
	for(GSList* node = aaf->children[arg]; node != NULL; node = node->next)
		if(taas__lab_get_label(lab,*(int*)node->data) == LAB_IN)
			return FALSE;
	int undec_attacker = FALSE;
	for(GSList* node = aaf->parents[arg]; node != NULL; node = node->next){
		int attacker_label = taas__lab_get_label(lab,*(int*)node->data);
		if(attacker_label == LAB_IN)
			return FALSE;
		if(attacker_label == LAB_UNDEC)
			undec_attacker = TRUE;
	}
	return undec_attacker;
}

/**
 * Checks wheter the argument "arg" is correctly labeled in
 * the given labeling (with the exception that arg_other is labelled lab_other)
 * wrt. the given AAF, i.e.
 * - if the argument is labeled "in" then all its attackers and all its
 *   attackees must be labeled "out"
 * - if the argument is labeled "out" then there must be at least one attacker
 *   labeled "in"
 * - if the argument is labeled "undec" then no attacker is labeled "in", there
 *   is at least one attacker labeled "undec", and no attackee is labeled "in"
 * This methods returns TRUE iff the argument is labeled correctly.
 */
int taas__labeled_correctly_under_assumption(struct AAF* aaf, struct Labeling* lab, int arg, int arg_other, int lab_other){
	int label = arg == arg_other ? lab_other : taas__lab_get_label(lab,arg);
	if(label == LAB_IN){
		// argument is "in"
		for(GSList* node = aaf->children[arg]; node != NULL; node = node->next)
			if(*(int*)node->data == arg_other && lab_other != LAB_OUT)
				return FALSE;
			else if(taas__lab_get_label(lab,*(int*)node->data) != LAB_OUT)
				return FALSE;
		for(GSList* node = aaf->parents[arg]; node != NULL; node = node->next)
			if(*(int*)node->data == arg_other && lab_other != LAB_OUT)
				return FALSE;
			else if(taas__lab_get_label(lab,*(int*)node->data) != LAB_OUT)
				return FALSE;
		return TRUE;
	}
	if(label == LAB_OUT){
		// argument is "out"
		for(GSList* node = aaf->parents[arg]; node != NULL; node = node->next)
			if(*(int*)node->data == arg_other && lab_other == LAB_IN)
				return TRUE;
			else if(taas__lab_get_label(lab,*(int*)node->data) == LAB_IN)
				return TRUE;
		return FALSE;
	}
	// argument is "undec"
	for(GSList* node = aaf->children[arg]; node != NULL; node = node->next)
		if(*(int*)node->data == arg_other && lab_other != LAB_IN)
			return FALSE;
		else if(taas__lab_get_label(lab,*(int*)node->data) == LAB_IN)
			return FALSE;
	int undec_attacker = FALSE;
	for(GSList* node = aaf->parents[arg]; node != NULL; node = node->next){
		int attacker_label = *(int*)node->data == arg_other ? lab_other : taas__lab_get_label(lab,*(int*)node->data);
		if(attacker_label == LAB_IN)
			return FALSE;
		if(attacker_label == LAB_UNDEC)
			undec_attacker = TRUE;
	}
	return undec_attacker;
}
/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
