/*
 * Copyright (C) 2021 QM Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef SERVED_PATH_VAR_MATCHER_HPP
#define SERVED_PATH_VAR_MATCHER_HPP

#include <string>

#include <served/mux/segment_matcher.hpp>

namespace served { namespace mux {

/*
 * Matches any non empty path segment.
 *
 * This segment matcher will match any non empty path segment, and captures the path segment as a
 * REST parameter.
 */
class variable_matcher : public segment_matcher
{
	const std::string _variable_name;

public:
	/*
	 * Constructs a variable matcher.
	 *
	 * The variable matcher contains a variable name for labelling REST parameters.
	 *
	 * @param variable_name the name of the REST param this segment matches.
	 */
	explicit variable_matcher(const std::string & variable);

	/*
	 * Checks that the path segment is not empty.
	 *
	 * @param path_segment the segment of path to check.
	 *
	 * @return true if the path segment is not empty, false otherwise
	 */
	virtual bool check_match(const std::string & path_segment) override;

	/*
	 * Appends any parameters extracted from the path segment to a list of params.
	 *
	 * This is used to propagate any REST parameters.
	 *
	 * @param params the list of parameters to append to
	 * @param path_segment the segment of path the variable should be extracted from
	 */
	virtual void get_param(served::parameters & params, const std::string & path_segment) override;
};

} } // mux, served

#endif // SERVED_PATH_VAR_MATCHER_HPP
