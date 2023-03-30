/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : taas-harper++.c
 Author      : Matthias Thimm
 Version     : 2.0
 Copyright   : GPL3
 Description : The taas-harper++ solver for abstract argumentation. This is an
               approximation solver for [DC-CO, DS-CO, DC-PR, DS-PR, DC-ST, DS-ST,
               DC-SST, DS-SST, DC-STG, DS-STG, DS-ID] that mainly uses the grounded extension
               to approximate answers. This version is based on taas-harper v2.0

============================================================================
*/
#define COMPUTATION_FINISHED 0
#define COMPUTATION_ABORTED__ANSWER_YES 1
#define COMPUTATION_ABORTED__ANSWER_NO  2
#define COMPUTATION_ABORTED__ANSWER_EMPTYSET  3
#define COMPUTATION_ABORTED__ANSWER_EMPTYEMPTYSET  4
#define COMPUTATION_FINISHED__EMPTY_GROUNDED  5
#define COMPUTATION_FINISHED__NONEMPTY_GROUNDED  6
#define COMPUTATION_FINISHED__ANSWER_NO  7

#define TRUE 1
#define FALSE 0
/* ============================================================================================================== */
/* ============================================================================================================== */
#include <stdio.h>
#include <ctype.h>
#include <cmath>
#include <glib.h>

#include "util/bitset.c"
#include "util/miscutil.c"

#include "taas/taas_aaf.c"
#include "taas/taas_inout.c"
#include "taas/taas_labeling.c"
#include "taas/taas_basics.c"

/* ============================================================================================================== */
/* ============================================================================================================== */

/**
 * Approximate answers
 */
void solve(struct TaskSpecification *task, struct AAF* aaf, struct Labeling* grounded){
  if(strcmp(task->problem,"DC") == 0 && strcmp(task->track,"DC-ID") != 0){
    // for credulous acceptance we always say "YES" iff the argument is not attacked by the grounded extension
    	if(bitset__get(grounded->out,task->arg)){
        printf("NO\n");
      }else printf("YES\n");
  }else{
    // for skeptical acceptance we always say "YES" iff the argument is in the grounded extension
    	if(bitset__get(grounded->in,task->arg)){
        printf("YES\n");
      }else printf("NO\n");
  }
  return;
}

/* ============================================================================================================== */
int main(int argc, char *argv[]){
  // General solver information
	struct SolverInformation *info = taas__solverinformation(
			"taas-harper++ v1.1.1 (2023-03-30)\nMatthias Thimm (matthias.thimm@fernuni-hagen.de)",
			"[i23]",
			"[DC-CO,DS-CO,DC-PR,DS-PR,DC-ST,DS-ST,DC-SST,DS-SST,DC-STG,DS-STG,DC-ID,DS-ID,DC-GR,DS-GR]"
		);
  return taas__solve(argc,argv,info,solve);
}

/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
