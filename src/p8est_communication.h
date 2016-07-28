/*
  This file is part of p4est.
  p4est is a C library to manage a collection (a forest) of multiple
  connected adaptive quadtrees or octrees in parallel.

  Copyright (C) 2010 The University of Texas System
  Written by Carsten Burstedde, Lucas C. Wilcox, and Tobin Isaac

  p4est is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  p4est is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with p4est; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef P8EST_COMMUNICATION_H
#define P8EST_COMMUNICATION_H

#include <p8est.h>

SC_EXTERN_C_BEGIN;

void                p8est_comm_parallel_env_create (p8est_t * p8est,
                                                    sc_MPI_Comm mpicomm);

void                p8est_comm_parallel_env_free (p8est_t * p8est);

int                 p8est_comm_parallel_env_is_null (p8est_t * p8est);

void                p8est_comm_parallel_env_assign (p8est_t * p8est,
                                                    sc_MPI_Comm mpicomm);

/** Caculate the number and partition of quadrents.
 * \param [in,out] p8est  Adds all \c p8est->local_num_quadrant counters and
 *                        puts cumulative sums in p4est->global_first_quadrant.
 */
void                p8est_comm_count_quadrants (p8est_t * p8est);

/** Distribute the global partition boundaries.
 * \param [in,out] p8est        Fills \c p8est->global_first_position.
 *                              p8est->first_local_tree must be set correctly.
 *                              If this processor is not empty and
 *                              first_quad is NULL, the first quadrant
 *                              of the first local tree must be set correctly.
 * \param [in] first_quad       If not NULL will be used as first quadrant.
 */
void                p8est_comm_global_partition (p8est_t * p8est,
                                                 p8est_quadrant_t *
                                                 first_quad);

/** Compute and distribute the cumulative number of quadrants per tree.
 * \param [in] p8est    This p8est needs to have correct values for
 *                      global_first_quadrant and global_first_position.
 * \param [in,out] pertree      On input, memory for num_trees + 1 numbers.
 *                              On output, the cumulative quadrant counts.
 */
void                p8est_comm_count_pertree (p8est_t * p8est,
                                              p4est_gloidx_t * pertree);

/** Query whether a processor has no quadrants.
 * \param [in] p8est    This forests' global_first_position array must be valid.
 * \param [in] p        Valid processor id.
 * \return              True if and only if processor \p is empty.
 */
int                 p8est_comm_is_empty (p8est_t * p8est, int p);

/** Tests ownershop of a quadrant via p8est->global_first_position.
 * Assumes a tree with no overlaps.
 * \param [in] rank    Rank whose ownership is tested.
 * \return true if rank is the owner.
 */
int                 p8est_comm_is_owner (p8est_t * p8est,
                                         p4est_locidx_t which_tree,
                                         const p8est_quadrant_t * q,
                                         int rank);

/** Searches the owner of a quadrant via p8est->global_first_position.
 * Assumes a tree with no overlaps.
 * \param [in] guess   Initial guess for the search.
 * \return Returns the processor id of the owner.
 */
int                 p8est_comm_find_owner (p8est_t * p8est,
                                           p4est_locidx_t which_tree,
                                           const p8est_quadrant_t * q,
                                           int guess);

/** Computes information about a tree being fully owned.
 * This is determined separately for the beginning and end of the tree.
 * \param [in] p8est            The p8est to work on.
 * \param [in] which_tree       The tree in question must be partially owned.
 * \param [out] full_tree[2]    Full ownership of beginning and end of tree.
 * \param [out] tree_contact[6] True if there are neighbors across the face.
 * \param [out] firstq          Smallest possible first quadrant on this core.
 * \param [out] nextq           Smallest possible first quadrant on next core.
 *                          Any of tree_contact, firstq and nextq may be NULL.
 */
void                p8est_comm_tree_info (p8est_t * p8est,
                                          p4est_locidx_t which_tree,
                                          int full_tree[],
                                          int tree_contact[],
                                          const p8est_quadrant_t ** firstq,
                                          const p8est_quadrant_t ** nextq);

/** Test if the 3x3 neighborhood of a quadrant is owned by this processor.
 * \param [in] p8est            The p8est to work on.
 * \param [in] which_tree       The tree index to work on.
 * \param [in] full_tree[2]     Flags as computed by p4est_comm_tree_info.
 * \param [in] tree_contact[6]  Flags as computed by p4est_comm_tree_info.
 * \param [in] q                The quadrant to be checked.
 * \return          Returns true iff this quadrant's 3x3 neighborhood is owned.
 */
int                 p8est_comm_neighborhood_owned (p8est_t * p8est,
                                                   p4est_locidx_t which_tree,
                                                   int full_tree[],
                                                   int tree_contact[],
                                                   p8est_quadrant_t * q);

/** Evaluates true/false of a flag among processors.
 * \param [in] p8est        The MPI communicator of this p8est will be used.
 * \param [in] flag         The variable to communicate.
 * \param [in] operation    Either sc_MPI_BAND or sc_MPI_BOR (not used bitwise).
 * \return          Returns the logical AND resp. OR of all processors' flags.
 */
int                 p8est_comm_sync_flag (p8est_t * p8est,
                                          int flag, sc_MPI_Op operation);

/** Compute a parallel checksum out of local checksums.
 * \param [in] p8est       The MPI information of this p8est will be used.
 * \param [in] local_crc   Locally computed adler32 checksum.
 * \param [in] local_bytes Number of bytes used for local checksum.
 * \return                 Parallel checksum on rank 0, 0 otherwise.
 */
unsigned            p8est_comm_checksum (p8est_t * p8est,
                                         unsigned local_crc,
                                         size_t local_bytes);

/** Context data to allow for split begin/end data transfer. */
typedef struct p8est_transfer_context
{
  int                 variable;
  int                 num_senders;
  int                 num_receivers;
  sc_MPI_Request     *recv_req;
  sc_MPI_Request     *send_req;
}
p8est_transfer_context_t;

/** Transfer data associated with one forest to a partitioned one.
 * In \ref p8est_partition, each quadrant's user data is transferred.
 * If the application maintains per-quadrant data outside of the forest,
 * this function can be used to transfer it, matching the call to partition.
 * This variant of the function assumes that the quadrant data size is fixed.
 * It sends point-to-point messages only and is blocking collective.
 * There is a split collective version; see the functions
 * \ref p8est_transfer_fixed_begin and \ref p8est_transfer_fixed_end.
 * \param [in] dest         This forest defines the target partition.
 *                          \b dest must have been derived from \b src by a
 *                          call to \ref p8est_partition.
 *                          It is legal to use \ref p8est_copy in the process.
 * \param [in] src          This forest defines the original partition.
 * \param [in] mpicomm      The communicator to use.  Must match \b
 *                          dest->mpicomm and \b src->mpicomm in size and rank.
 * \param [in] tag          This tag is used in all messages.  The user must
 *                          guarantee that \b mpicomm and \b tag do not
 *                          conflict with other messages in transit.
 * \param [out] dest_data   User-allocated memory of size \b data_size * \b
 *                          dest->local_num_quadrants is received into.
 * \param [in] src_data     User-allocated memory of size \b data_size * \b
 *                          src->local_num_quadrants bytes is sent from.
 * \param [in] data_size    Fixed data size per quadrant.
 */
void                p8est_transfer_fixed (p8est_t * dest, p8est_t * src,
                                          sc_MPI_Comm mpicomm, int tag,
                                          void *dest_data,
                                          const void *src_data,
                                          size_t data_size);

/** Initiate a fixed-size data transfer between partitions.
 * See \ref p8est_transfer_fixed for a full description.
 * Must be matched with \ref p8est_transfer_fixed_end for completion.
 * All parameters must stay alive until the completion has been called.
 * \param [in] dest         This forest defines the target partition.
 *                          \b dest must have been derived from \b src by a
 *                          call to \ref p8est_partition.
 *                          It is legal to use \ref p8est_copy in the process.
 * \param [in] src          This forest defines the original partition.
 * \param [in] mpicomm      The communicator to use.  Must match \b
 *                          dest->mpicomm and \b src->mpicomm in size and rank.
 * \param [in] tag          This tag is used in all messages.  The user must
 *                          guarantee that \b mpicomm and \b tag do not
 *                          conflict with other messages in transit.
 * \param [out] dest_data   User-allocated memory of size \b data_size * \b
 *                          dest->local_num_quadrants bytes is received into.
 *                          It must not be accessed before completion with
 *                          \ref p8est_transfer_fixed_end.
 * \param [in] src_data     User-allocated memory of size \b data_size * \b
 *                          src->local_num_quadrants bytes is sent from.
 *                          It must not be accessed before completion with
 *                          \ref p8est_transfer_fixed_end.
 * \param [in] data_size    Fixed data size per quadrant.
 * \return                  The context object must be passed to the matching
 *                          call to \ref p8est_transfer_fixed_end.
 */
p8est_transfer_context_t *p8est_transfer_fixed_begin (p8est_t * dest,
                                                      p8est_t * src,
                                                      sc_MPI_Comm mpicomm,
                                                      int tag,
                                                      void *dest_data,
                                                      const void *src_data,
                                                      size_t data_size);

/** Complete a fixed-size data transfer between partitions.
 * \param [in] tc       Context data from \ref p8est_transfer_fixed_begin.
 *                      Is deallocated before this function returns.
 */
void                p8est_transfer_fixed_end (p8est_transfer_context_t * tc);

/** Transfer variable-size quadrant data between partitions.
 * (See \ref p8est_transfer_fixed that is optimized for fixed-size data.)
 * The destination process may not know the data size for the elements it
 * receives.  In this case the sizes need to be obtained separately in advance,
 * for example by calling \ref p8est_transfer_fixed with \b src_sizes as
 * payload data, or alternatively its split begin/end versions.
 * \param [in] dest         This forest defines the target partition.
 *                          \b dest must have been derived from \b src by a
 *                          call to \ref p8est_partition.
 *                          It is legal to use \ref p8est_copy in the process.
 * \param [in] src          This forest defines the original partition.
 * \param [in] mpicomm      The communicator to use.  Must match \b
 *                          dest->mpicomm and \b src->mpicomm in size and rank.
 * \param [in] tag          This tag is used in all messages.  The user must
 *                          guarantee that \b mpicomm and \b tag do not
 *                          conflict with other messages in transit.
 * \param [out] dest_data   User-allocated memory of
 *                          sum_{i in \b dest->local_num_quadrants} \b
 *                          dest_sizes [i] many bytes is received into.
 *                          See below about how to choose its size.
 * \param [in] dest_sizes   User-allocated memory of one integer for each
 *                          quadrant, storing the data size to receive for it.
 *                          We understand that the sizes are often not known a
 *                          priori, in which case they can be obtained by a
 *                          prior call to \ref p8est_transfer_fixed.
 *                          Optionally the split begin/end versions can be used
 *                          for added flexibility and overlapping of messages.
 *                          We use the type int to minimize the message size,
 *                          and to conform to MPI that has no type for size_t.
 * \param [in] src_data     User-allocated memory of
 *                          sum_{i in \b src->local_num_quadrants} \b
 *                          src_sizes [i] many bytes is sent from.
 * \param [in] src_sizes    User-allocated memory of one integer for each
 *                          quadrant, storing the data size to send for it.
 *                          We use the type int to minimize the message size,
 *                          and to conform to MPI that has no type for size_t.
 */
void                p8est_transfer_custom (p8est_t * dest, p8est_t * src,
                                           sc_MPI_Comm mpicomm, int tag,
                                           void *dest_data,
                                           const int *dest_sizes,
                                           const void *src_data,
                                           const int *src_sizes);

/** Initiate a variable-size data transfer between partitions.
 * See \ref p8est_transfer_custom for a full description.
 * Must be matched with \ref p8est_transfer_custom_end for completion.
 * All parameters must stay alive until the completion has been called.
 * \param [in] dest         This forest defines the target partition.
 *                          \b dest must have been derived from \b src by a
 *                          call to \ref p8est_partition.
 *                          It is legal to use \ref p8est_copy in the process.
 * \param [in] src          This forest defines the original partition.
 * \param [in] mpicomm      The communicator to use.  Must match \b
 *                          dest->mpicomm and \b src->mpicomm in size and rank.
 * \param [in] tag          This tag is used in all messages.  The user must
 *                          guarantee that \b mpicomm and \b tag do not
 *                          conflict with other messages in transit.
 * \param [out] dest_data   User-allocated memory of
 *                          sum_{i in \b dest->local_num_quadrants} \b
 *                          dest_sizes [i] many bytes is received into.
 *                          It must not be accessed before completion with
 *                          \ref p8est_transfer_custom_end.
 *                          See below about how to choose its size.
 * \param [in] dest_sizes   User-allocated memory of one integer for each
 *                          quadrant, storing the data size to receive for it.
 *                          We understand that the sizes are often not known a
 *                          priori, in which case they can be obtained by a
 *                          prior call to \ref p8est_transfer_fixed.
 *                          Optionally the split begin/end versions can be used
 *                          for added flexibility and overlapping of messages.
 *                          We use the type int to minimize the message size,
 *                          and to conform to MPI that has no type for size_t.
 * \param [in] src_data     User-allocated memory of
 *                          sum_{i in \b src->local_num_quadrants} \b
 *                          src_sizes [i] many bytes is sent from.
 *                          It must not be accessed before completion with
 *                          \ref p8est_transfer_custom_end.
 * \param [in] src_sizes    User-allocated memory of one integer for each
 *                          quadrant, storing the data size to send for it.
 *                          We use the type int to minimize the message size,
 *                          and to conform to MPI that has no type for size_t.
 * \return                  The context object must be passed to the matching
 *                          call to \ref p8est_transfer_custom_end.
 */
p8est_transfer_context_t *p8est_transfer_custom_begin (p8est_t * dest,
                                                       p8est_t * src,
                                                       sc_MPI_Comm mpicomm,
                                                       int tag,
                                                       void *dest_data,
                                                       const int *dest_sizes,
                                                       const void *src_data,
                                                       const int *src_sizes);

/** Complete a variable-size data transfer between partitions.
 * \param [in] tc       Context data from \ref p8est_transfer_custom_begin.
 *                      Is deallocated before this function returns.
 */
void                p8est_transfer_custom_end (p8est_transfer_context_t * tc);

SC_EXTERN_C_END;

#endif /* !P8EST_COMMUNICATION_H */
