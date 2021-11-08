/*
 * Copyright (C) QM Ltd.
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

#include <test/catch.hpp>

#include <served/status.hpp>
#include <served/request_error.hpp>

TEST_CASE("Test request error", "[request_error]")
{
	try
	{
		throw served::request_error(
			served::status_5XX::INTERNAL_SERVER_ERROR,
			"internal error");
	}
	catch (served::request_error const& e)
	{
		REQUIRE( std::string(e.what()) == "internal error" );
		REQUIRE( e.get_status_code() == served::status_5XX::INTERNAL_SERVER_ERROR);
		REQUIRE( e.get_content_type() == "text/plain" );
	}

	try
	{
		throw served::request_error(
			served::status_4XX::NOT_FOUND,
			"{\"error\":\"File not found\"}",
			"application/json");
	}
	catch (served::request_error const& e)
	{
		REQUIRE( std::string(e.what()) == "{\"error\":\"File not found\"}" );
		REQUIRE( e.get_status_code() == served::status_4XX::NOT_FOUND );
		REQUIRE( e.get_content_type() == "application/json" );
	}
}
