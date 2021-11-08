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

#ifndef SERVED_PATH_EMPTY_MATCHER_HPP
#define SERVED_PATH_EMPTY_MATCHER_HPP

#include <string>

#include <served/mux/segment_matcher.hpp>

namespace served { namespace mux {

/*
 * Matches all segments.
 *
 * This segment matcher will match any path segment. It is used when a path ends with "/".
 */
class empty_matcher : public segment_matcher
{
public:
	/*
	 * Checks whether the path segment matches this segment matcher.
	 *
	 * @param path_segment the path segment to check matching against.
	 *
	 * @return always true
	 */
	virtual bool check_match(const std::string &) override
	{
		return true;
	}

	/*
	 * Appends any parameters extracted from the path segment to a list of params.
	 *
	 * This is used to propagate any REST parameters.
	 *
	 * @param params the list of parameters to append to
	 * @param path_segment the segment of path the variable should be extracted from
	 */
	virtual void get_param(served::parameters &, const std::string &) override
	{
	}
};

} } // mux, served

#endif // SERVED_PATH_EMPTY_MATCHER_HPP
