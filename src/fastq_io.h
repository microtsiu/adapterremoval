/*************************************************************************\
 * AdapterRemoval - cleaning next-generation sequencing reads            *
 *                                                                       *
 * Copyright (C) 2015 by Mikkel Schubert - mikkelsch@gmail.com           *
 *                                                                       *
 * If you use the program, please cite the paper:                        *
 * S. Lindgreen (2012): AdapterRemoval: Easy Cleaning of Next Generation *
 * Sequencing Reads, BMC Research Notes, 5:337                           *
 * http://www.biomedcentral.com/1756-0500/5/337/                         *
 *                                                                       *
 * This program is free software: you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
\*************************************************************************/
#ifndef FASTQ_IO_H
#define FASTQ_IO_H

#include <vector>
#include <memory>

#include "commontypes.h"
#include "fastq.h"
#include "scheduler.h"
#include "timer.h"


class userconfig;


/**
 * Container object for raw and trimmed, collapsed, etc. reads.
 */
class fastq_file_chunk : public analytical_chunk
{
public:
    /** Create chunk representing lines starting at line offset (1-based). */
    fastq_file_chunk(size_t offset_);

    //! The line-number offset from which the lines start
    size_t offset;

    //! Lines read from the mate 1 and mate 2 files
    std::vector<string_vec> mates;
    //! Lines to write to output files.
    std::vector<string_vec> output;
};


/**
 * Simple file reading step.
 *
 * Reads from either the mate 1 or the mate 2 file, storing the reads in the
 * mates variable of a fastq_file_chunk, using the index corresponding to
 * either rt_mate_1 or rt_mate_2. Once the EOF has been reached, a single
 * empty of lines will be returned.
 *
 * The class will re-use existing fastq_file_chunk objects passed to the
 * 'process' function, resizing the list of lines as nessesary to match the
 * number of lines read.
 */
class read_paired_fastq : public analytical_step
{
public:
    /**
     * Constructor.
     *
     * @param config User settings; needed for 'open_ifstream'.
     * @param mate Either rt_mate_1 or rt_mate_2; other values throw.
     *
     * Opens the input file corresponding to the specified mate.
     */
    read_paired_fastq(const userconfig& config, read_type mate);

    /** Reads N lines from the input file and saves them in an fastq_file_chunk. */
    virtual analytical_chunk* process(analytical_chunk* chunk);

private:
    //! Current line in the input file (1-based)
    size_t m_line_offset;
    //! Pointer to iostream opened using userconfig::open_ifstream
    std::auto_ptr<std::istream> m_io_input;
    //! Read type; either rt_mate_1 or rt_mate_2.
    const read_type m_type;
};


/**
 * Simple file reading step.
 *
 * The 'process' function takes a fastq_file_chunk object and writes the lines
 * at the offset corresponding to the 'type' argument to the corresponding
 * output file. The list of lines is cleared upon writing.
 */
class write_paired_fastq : public analytical_step
{
public:
    /**
     * Constructor.
     *
     * @param config User settings.
     * @param read_type The type of reads to write.
     * @param progress Print progress reports using a 'timer' object.
     *
     * Based on the read-type specified, and SE / PE mode, the corresponding
     * output file is opened
     */
    write_paired_fastq(const userconfig& config, read_type type, bool progress = false);

    /** Destructor; closes output file. */
    ~write_paired_fastq();

    /** Writes the reads of the type specified in the constructor. */
    virtual analytical_chunk* process(analytical_chunk* chunk);

    /** Flushes the output file and prints progress report (if enabled). */
    virtual void finalize();

private:
    /** Writes the given lines to file, as is; no new-lines are added. */
	static void write_lines(std::auto_ptr<std::ostream>& file, string_vec& lines);

    const read_type m_type;
    //! When true, progress reports are printed using the 'm_timer' object.
    const bool m_progress;
    //! Pointer to output file opened using userconfig::open_with_default_filename.
    std::auto_ptr<std::ostream> m_output;
    //! Timer for optional progress reporting; only used if 'progress' is set.
    timer m_timer;
};

#endif
