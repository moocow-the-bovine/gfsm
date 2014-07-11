
/*=============================================================================*\
 * File: gfsmTrain.h
 * Author: Bryan Jurish <moocow.bovine@gmail.com>
 * Description: finite state machine library
 *
 * Copyright (c) 2014 Bryan Jurish.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *=============================================================================*/

/** \file gfsmTrain.h
 *  \brief transducer training from (input,output) string pairs
 */

#ifndef _GFSM_TRAIN_H
#define _GFSM_TRAIN_H

#include <gfsmAutomaton.h>
#include <gfsmUtils.h>

/*======================================================================
 * Types: train
 */
/** \brief top-level type for automaton training data and options */
typedef struct {
  /** 
   * if true, only minimal-cost path(s) will be considered for each training pair.
   */
  gboolean bestPathsOnly;

  /** 
   * if true, only unique successful paths modulo arc-ordering will be considered;
   * e.g. (q --[<epsilon>:a]--> q --[a:<epsilon>]--> q) and (q --[a:<epsilon>]--> q --[<epsilon>:a]--> q)
   * are duplicates in this sense, since they differ only in the ordering of the arcs.
   */
  gboolean prunePathPermutations;

  /** 
   * If true, a total count-mass of 1 will be added for each (input,output) pair,
   * and distributed uniformly among any successful paths for that pair.
   * Otherwise, each successful path for a given pair will receive a count-mass of 1 (one).
   */
  gboolean distributeOverPaths;

  /** 
   * if true, the total count-mass added to each successful path will
   * be distributed uniformly over all its arcs and its final weight.
   * Otherwise, each arc in the path will receive the full count-mass alotted to that path.
   */
  gboolean distributeOverArcs;

  /** 
   * source transducer to use for training; weights are indices into ::gfsmTrainer::weights
   */
  gfsmAutomaton *fst;

  /** 
   * number of weights allocated in weights, counts
   */
  guint nweights;

  /** 
   * vector of all weights used by source transducer; GArray* of gfsmWeight
   */
  gfsmWeight *weights;

  /** 
   * vector of accumulated training counts corresponding 1-1 to weights; GArray* of gfsmWeight
   */
  gfsmWeight *counts;

} gfsmTrainer;


/** \brief low-level type for gfsm_automaton_train() computation state */
typedef struct gfsmTrainConfig_ {
  guint32     i;   /**< current position in input vector */
  guint32     j;   /**< current position in output vector */
  gfsmStateId qf;  /**< final state id, or gfsmNoState for non-final state */
  gfsmArc*    a;   /**< last followed arc, or NULL for initial and/or final config */
  struct gfsmTrainConfig_ *prev; /**< previous config in this path */
} gfsmTrainConfig;

/** \brief low-level type for a successful training-path: GPtrArray* of gfsmTrainConfig* */
typedef GPtrArray gfsmTrainPath;


/*======================================================================
 * Methods: training: top-level
 */

//------------------------------
/** 
 * allocate a new gfsmTrainer for training source FST \a fst
 * \param fst automaton to be trained: it will be destructively altered by this call (weights changed to indices)
 * \returns new gfsmTrainer
 */
gfsmTrainer* gfsm_trainer_new(gfsmAutomaton *fst);

//------------------------------
/** free a gfsmTrainer 
 * \param free_fst if true, the underlying FST will also be freed if still present.
 */
void gfsm_trainer_free(gfsmTrainer *trainer, gboolean free_fst);

//------------------------------
/** extract an automaton representing the training counts from a ::gfsmTrainer \a trainer.
 * \param trainer trainer from which to extract FST topology and counts
 * \param steal if true, \a trainer->fst will be used directly, invalidating \a trainer for any futher
 *        calls except for gfsm_trainer_free().
 * \returns modified \a trainer->fst if \a steal is true, otherwise a clone whose weights are counts
 *          as stored in \a trainer->counts
 */
gfsmAutomaton* gfsm_trainer_automaton(gfsmTrainer *trainer, gboolean steal);

//------------------------------
/** transducer weight-training utility:
 *  count successful paths in training FST for the i/o pair (\a input, \a output),
 *  updating \a trainer->counts corresponding to \a fst arc- and final-weights.
 *  On completion, each count is the (real)-sum of its original value and the
 *  number of occurrences of the corresponding arc (rsp. final state) in any successful path in \a fst
 *  with labels (\a input, \a output).  May be run multiple times on the same trainer to add
 *  counts for multiple (\a input, \a output) pairs.
 *  \param trainer transducer trainer, which should have been initialized as by gfsm_trainer_new()
 *  \param input input labels (lower)
 *  \param output output labels (upper)
 */
void gfsm_trainer_train(gfsmTrainer *trainer, gfsmLabelVector *input, gfsmLabelVector *output);

#endif /* _GFSM_TRAIN_H */
