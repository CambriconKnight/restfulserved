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

#include <cstring>

#include <served/methods.hpp>
#include <served/request_parser_impl.hpp>

TEST_CASE("request parser impl can parse http requests", "[request_parser_impl]")
{
	served::request req;
	served::request_parser_impl parser(req);
	const char* request =
		"POST /you/got/served?reason=science#idet HTTP/1.1\r\n"
		"Host: api.datasift.com\r\n"
		"Content-Type: text/xml; charset=utf-8\r\n"
		"Content-Length: 15\r\n"
		"X-Example-Dup: val1\r\n"
		"X-Example-Dup: val2\r\n"
		"X-Example-Dup: val3\r\n"
		"\r\n"
		"you got served!";

	auto status = parser.parse(request, strlen(request));

	REQUIRE(status == served::request_parser_impl::FINISHED);

	SECTION("header is parsed correctly")
	{
		SECTION("check request")
		{
			REQUIRE(req.method()       == served::method::POST);
			REQUIRE(req.HTTP_version() == "HTTP/1.1");
			REQUIRE(req.body()         == "you got served!");
		}
		SECTION("check uri")
		{
			REQUIRE(req.url().URI()      == "/you/got/served?reason=science");
			REQUIRE(req.url().path()     == "/you/got/served");
			REQUIRE(req.url().query()    == "reason=science");
			REQUIRE(req.url().fragment() == "idet");
		}
		SECTION("check query")
		{
			REQUIRE(req.query["reason"] == "science");
		}
		SECTION("check fields")
		{
			REQUIRE(req.header("Host")           == "api.datasift.com");
			REQUIRE(req.header("Content-Type")   == "text/xml; charset=utf-8");
			REQUIRE(req.header("CONTENT-LENGTH") == "15");
			REQUIRE(req.header("X-EXAMPLE-DUP")  == "val1,val2,val3");
		}
	}
}

TEST_CASE("request parser read partial http requests", "[request_parser_impl]")
{
	const char* full_request =
		"POST /you/got/served?reason=science#idet HTTP/1.1\r\n"
		"Host: api.datasift.com\r\n"
		"Content-Type: text/xml; charset=utf-8\r\n"
		"Content-Length: 15\r\n"
		"X-Example-Dup: val1\r\n"
		"X-Example-Dup: val2\r\n"
		"X-Example-Dup: val3\r\n"
		"\r\n"
		"you got served!";

	// Simulate scenarios where a request headers overpass the size of a single
	// buffer ( > 8192 bytes ). This might happen arbitrarily if a request header
	// size > 8192bytes OR if the TCP stack decides to give us the HTTP headers in
	// two callback calls instead of one.
	//
	// This can be exploited as a security vulnerability as an attacker could
	// craft a malicious HTTP request inside a valid HTTP request (authorized by a
	// reverse-proxy) if the attacker uses a smart packet split.
	//
	// It can also be considered as a conventional bug where every request with a
	// path > buffer size will trigger arbitrary backend handler with a corrupted
	// URI path.
	//
	std::vector<std::string::size_type> break_points = {{
		2, 22, 42, 60, 72, 100, 128, 160, 170, 196, 208,
	}};

	for ( const auto & break_point : break_points )
	{
		served::request req;
		served::request_parser_impl parser(req);

		auto request1 = std::string(full_request, break_point);
		auto request2 = std::string(full_request+break_point, strlen(full_request)-break_point);

		INFO("Break point: " << break_point);

		auto status = parser.parse(request1.data(), request1.length());

		REQUIRE((
			status == served::request_parser_impl::READ_HEADER ||
			status == served::request_parser_impl::READ_BODY
		));

		status = parser.parse(request2.data(), request2.length());

		REQUIRE(status == served::request_parser_impl::FINISHED);

		SECTION("header is parsed correctly")
		{
			SECTION("check request")
			{
				CHECK(req.method()       == served::method::POST);
				CHECK(req.HTTP_version() == "HTTP/1.1");
				CHECK(req.body()         == "you got served!");
			}
			SECTION("check uri")
			{
				CHECK(req.url().URI()      == "/you/got/served?reason=science");
				CHECK(req.url().path()     == "/you/got/served");
				CHECK(req.url().query()    == "reason=science");
				CHECK(req.url().fragment() == "idet");
			}
			SECTION("check query")
			{
				CHECK(req.query["reason"] == "science");
			}
			SECTION("check fields")
			{
				CHECK(req.header("Host")           == "api.datasift.com");
				CHECK(req.header("Content-Type")   == "text/xml; charset=utf-8");
				CHECK(req.header("CONTENT-LENGTH") == "15");
				CHECK(req.header("X-EXAMPLE-DUP")  == "val1,val2,val3");
			}
		}
	}
}

TEST_CASE("request parser impl can parse bad requests", "[request_parser_impl]")
{
	SECTION("Bad HTTP method")
	{
		served::request req;
		served::request_parser_impl parser(req);
		const char* request =
			"OGERTY /you/got/served HTTP/1.1\r\n"
			"Host: api.datasift.com\r\n"
			"Content-Type: text/xml; charset=utf-8\r\n"
			"Content-Length: 15\r\n"
			"\r\n"
			"you got played!";

		auto status = parser.parse(request, strlen(request));

		REQUIRE(status == served::request_parser_impl::ERROR);
	}

	SECTION("Unrecognised HTTP protocol")
	{
		served::request req;
		served::request_parser_impl parser(req);
		const char* request =
			"POST /you/got/served HTTPZ/-09\r\n"
			"Host: api.datasift.com\r\n"
			"Content-Type: text/xml; charset=utf-8\r\n"
			"Content-Length: 15\r\n"
			"\r\n"
			"you got played!";

		auto status = parser.parse(request, strlen(request));

		REQUIRE(status == served::request_parser_impl::ERROR);
	}
}

TEST_CASE("request parser impl can handle utf-8", "[request_parser_impl]")
{
	served::request req;
	served::request_parser_impl parser(req);
	const char* request =
		u8"POST /you/got/served?reason=science&reason2=theinternet&reason%25=the%24%24#idet HTTP/1.1\r\n"
		u8"Host: api.datasift.com\r\n"
		u8"Content-Type: text/xml; charset=utf-8\r\n"
		u8"Content-Length: 22\r\n"
		u8"\r\n"
		u8"Unicode character: \u2018";

	auto status = parser.parse(request, strlen(request));

	REQUIRE(status == served::request_parser_impl::FINISHED);

	SECTION("header is parsed correctly")
	{
		SECTION("check request")
		{
			REQUIRE(req.method()       == served::method::POST);
			REQUIRE(req.HTTP_version() == u8"HTTP/1.1");
			REQUIRE(req.body()         == u8"Unicode character: \u2018");
		}
		SECTION("check uri")
		{
			REQUIRE(req.url().URI()      == u8"/you/got/served?reason=science&reason2=theinternet&reason%25=the%24%24");
			REQUIRE(req.url().path()     == u8"/you/got/served");
			REQUIRE(req.url().query()    == u8"reason=science&reason2=theinternet&reason%25=the%24%24");
			REQUIRE(req.url().fragment() == u8"idet");
		}
		SECTION("check query")
		{
			REQUIRE(req.query["reason"] == u8"science");
			REQUIRE(req.query["reason2"] == u8"theinternet");
			REQUIRE(req.query["reason%"] == u8"the$$");
		}
		SECTION("check fields")
		{
			REQUIRE(req.header("HOST")           == u8"api.datasift.com");
			REQUIRE(req.header("CONTENT-TYPE")   == u8"text/xml; charset=utf-8");
			REQUIRE(req.header("CONTENT-length") == u8"22");
		}
	}
}


TEST_CASE("test parser states", "[request_parser_impl]")
{
	typedef served::request_parser_impl::status_type status_type;
	typedef std::tuple<std::string, status_type>     section_story;
	typedef std::vector<section_story>               section_stories;

	SECTION("GET req with no body")
	{
		served::request dummy_req;
		served::request_parser_impl parser(dummy_req);
		auto sections = section_stories {{
			section_story { "GET /endpoints/int/test HTTP/1.1\r\n", status_type::READ_HEADER },
			section_story { "Host: localhost\r\n",                  status_type::READ_HEADER },
			section_story { "Agent: me\r\n",                        status_type::READ_HEADER },
			section_story { "\r\n",                                 status_type::FINISHED    },
			section_story { "this should be ig",                    status_type::FINISHED    },
			section_story { "nored entirely",                       status_type::FINISHED    },
		}};

		for ( const auto & section : sections )
		{
			const std::string s = std::get<0>(section);
			INFO("section: " << s);
			REQUIRE(std::get<1>(section) == parser.parse(s.c_str(), s.length()));
		}

		CHECK(dummy_req.method() == served::method::GET);
		CHECK(dummy_req.url().path() == "/endpoints/int/test");
		CHECK(dummy_req.url().query() == "");
		CHECK(dummy_req.header("host") == "localhost");
		CHECK(dummy_req.header("agent") == "me");
	}

	SECTION("GET bad request")
	{
		served::request dummy_req;
		served::request_parser_impl parser(dummy_req);
		auto sections = section_stories {{
			section_story { "GET /endpoinMISTAKEHEREHTTP/1.1\r", status_type::ERROR },
			section_story { "\nHost: localhost",                 status_type::ERROR },
			section_story { "\r\nAgent: me\r\n",                 status_type::ERROR },
			section_story { "\r\n",                              status_type::ERROR },
		}};

		for ( const auto & section : sections )
		{
			const std::string s = std::get<0>(section);
			INFO("section: " << s);
			REQUIRE(std::get<1>(section) == parser.parse(s.c_str(), s.length()));
		}
	}

	/*
	TODO: Improve query string parsing so that these tests pass.

	SECTION("GET bad request query")
	{
		served::request dummy_req;
		served::request_parser_impl parser(dummy_req);
		auto sections = section_stories {{
			section_story { "GET /endpoint??thisiswrong& HTTP/1.1\r", status_type::ERROR },
			section_story { "\nHost: localhost",                      status_type::ERROR },
			section_story { "\r\nAgent: me\r\n",                      status_type::ERROR },
			section_story { "\r\n",                                   status_type::ERROR },
		}};

		for ( const auto & section : sections )
		{
			const std::string s = std::get<0>(section);
			INFO("section: " << s);
			REQUIRE(std::get<1>(section) == parser.parse(s.c_str(), s.length()));
		}
	}

	SECTION("GET bad request query 2")
	{
		served::request dummy_req;
		served::request_parser_impl parser(dummy_req);
		auto sections = section_stories {{
			section_story { "GET /endpoint?t=r&wrong HTTP/1.1\r", status_type::ERROR },
			section_story { "\nHost: localhost",                  status_type::ERROR },
			section_story { "\r\nAgent: me\r\n",                  status_type::ERROR },
			section_story { "\r\n",                               status_type::ERROR },
		}};

		for ( const auto & section : sections )
		{
			const std::string s = std::get<0>(section);
			INFO("section: " << s);
			REQUIRE(std::get<1>(section) == parser.parse(s.c_str(), s.length()));
		}
	}
	*/

	SECTION("POST with body")
	{
		served::request dummy_req;
		served::request_parser_impl parser(dummy_req);
		auto sections = section_stories {{
			section_story { "POST /endpoints HTTP/1.1\r\n", status_type::READ_HEADER },
			section_story { "Content-Type: text/html\r\n",  status_type::READ_HEADER },
			section_story { "Content-Length: 40\r\n",       status_type::READ_HEADER },
			section_story { "\r\nA small amoun",            status_type::READ_BODY   },
			section_story { "t of body for you",            status_type::READ_BODY   },
			section_story { "to enjoy plz thxx",            status_type::FINISHED    },
			section_story { "plz ignore this..",            status_type::FINISHED    },
		}};

		for ( const auto & section : sections )
		{
			const std::string s = std::get<0>(section);
			INFO("section: " << s);
			REQUIRE(std::get<1>(section) == parser.parse(s.c_str(), s.length()));
		}

		CHECK(dummy_req.body() == "A small amount of body for youto enjoy p");
	}

	SECTION("PUT with body")
	{
		served::request dummy_req;
		served::request_parser_impl parser(dummy_req);
		auto sections = section_stories {{
			section_story { "PUT /endpoints HTTP/1.1\r\n", status_type::READ_HEADER },
			section_story { "Content-Type: text/html\r\n", status_type::READ_HEADER },
			section_story { "Content-Length: 40\r\n",      status_type::READ_HEADER },
			section_story { "\r\nA small amoun",           status_type::READ_BODY   },
			section_story { "t of body for you",           status_type::READ_BODY   },
			section_story { "to enjoy plz thxx",           status_type::FINISHED    },
			section_story { "plz ignore this..",           status_type::FINISHED    },
		}};

		for ( const auto & section : sections )
		{
			const std::string s = std::get<0>(section);
			INFO("section: " << s);
			REQUIRE(std::get<1>(section) == parser.parse(s.c_str(), s.length()));
		}

		CHECK(dummy_req.body() == "A small amount of body for youto enjoy p");
	}

	SECTION("PUT without content type")
	{
		served::request dummy_req;
		served::request_parser_impl parser(dummy_req);
		auto sections = section_stories {{
			section_story { "PUT /endpoints HTTP/1.1\r\n", status_type::READ_HEADER },
			section_story { "Content-Length: 40\r\n",      status_type::READ_HEADER },
			section_story { "\r\nA small amoun",           status_type::FINISHED    },
			section_story { "t of body for you",           status_type::FINISHED    },
			section_story { "to enjoy plz thxx",           status_type::FINISHED    },
			section_story { "plz ignore this..",           status_type::FINISHED    },
		}};

		for ( const auto & section : sections )
		{
			const std::string s = std::get<0>(section);
			INFO("section: " << s);
			REQUIRE(std::get<1>(section) == parser.parse(s.c_str(), s.length()));
		}

		CHECK(dummy_req.body() == "");
	}

	SECTION("PUT without content length")
	{
		served::request dummy_req;
		served::request_parser_impl parser(dummy_req);
		auto sections = section_stories {{
			section_story { "PUT /endpoints HTTP/1.1\r\n", status_type::READ_HEADER },
			section_story { "Content-Type: text/html\r\n", status_type::READ_HEADER },
			section_story { "\r\nA small amoun",           status_type::FINISHED    },
			section_story { "t of body for you",           status_type::FINISHED    },
			section_story { "to enjoy plz thxx",           status_type::FINISHED    },
			section_story { "plz ignore this..",           status_type::FINISHED    },
		}};

		for ( const auto & section : sections )
		{
			const std::string s = std::get<0>(section);
			INFO("section: " << s);
			REQUIRE(std::get<1>(section) == parser.parse(s.c_str(), s.length()));
		}

		CHECK(dummy_req.body() == "");
	}

	SECTION("PUT with 100-continue")
	{
		served::request dummy_req;
		served::request_parser_impl parser(dummy_req);
		auto sections = section_stories {{
			section_story { "PUT /endpoints HTTP/1.1\r\n", status_type::READ_HEADER     },
			section_story { "Content-Type: text/html\r\n", status_type::READ_HEADER     },
			section_story { "Content-Length: 40\r\n",      status_type::READ_HEADER     },
			section_story { "Expect: 100-continue\r\n",    status_type::READ_HEADER     },
			section_story { "\r\n",                        status_type::EXPECT_CONTINUE },
			section_story { "A small amount of body f",    status_type::READ_BODY       },
			section_story { "or youto enjoy plz thxx",     status_type::FINISHED        },
			section_story { "plz ignore this..",           status_type::FINISHED        },
		}};

		for ( const auto & section : sections )
		{
			const std::string s = std::get<0>(section);
			INFO("section: " << s);
			REQUIRE(std::get<1>(section) == parser.parse(s.c_str(), s.length()));
		}

		CHECK(dummy_req.body() == "A small amount of body for youto enjoy p");
	}

	SECTION("PUT with 100-continue no content length")
	{
		served::request dummy_req;
		served::request_parser_impl parser(dummy_req);
		auto sections = section_stories {{
			section_story { "PUT /endpoints HTTP/1.1\r\n", status_type::READ_HEADER },
			section_story { "Content-Type: text/html\r\n", status_type::READ_HEADER },
			section_story { "Expect: 100-continue\r\n",    status_type::READ_HEADER },
			section_story { "\r\n",                        status_type::ERROR       },
			section_story { "A small amount of body f",    status_type::ERROR       },
			section_story { "or youto enjoy plz thxx",     status_type::ERROR       },
			section_story { "plz ignore this..",           status_type::ERROR       },
		}};

		for ( const auto & section : sections )
		{
			const std::string s = std::get<0>(section);
			INFO("section: " << s);
			REQUIRE(std::get<1>(section) == parser.parse(s.c_str(), s.length()));
		}
	}

	SECTION("POST under limit")
	{
		served::request dummy_req;
		served::request_parser_impl parser(dummy_req, 122);
		auto sections = section_stories {{
			section_story { "POST /endpoints HTTP/1.1\r\n", status_type::READ_HEADER },
			section_story { "Content-Type: text/html\r\n",  status_type::READ_HEADER },
			section_story { "Content-Length: 40\r\n",       status_type::READ_HEADER },
			section_story { "\r\nA small amoun",            status_type::READ_BODY   },
			section_story { "t of body for you",            status_type::READ_BODY   },
			section_story { "to enjoy plz thxx",            status_type::FINISHED    },
		}};

		for ( const auto & section : sections )
		{
			const std::string s = std::get<0>(section);
			INFO("section: " << s);
			REQUIRE(std::get<1>(section) == parser.parse(s.c_str(), s.length()));
		}

		CHECK(dummy_req.body() == "A small amount of body for youto enjoy p");
	}

	SECTION("POST over limit")
	{
		served::request dummy_req;
		served::request_parser_impl parser(dummy_req, 115);
		auto sections = section_stories {{
			section_story { "POST /endpoints HTTP/1.1\r\n", status_type::READ_HEADER           },
			section_story { "Content-Type: text/html\r\n",  status_type::READ_HEADER           },
			section_story { "Content-Length: 40\r\n",       status_type::READ_HEADER           },
			section_story { "\r\nA small amoun",            status_type::READ_BODY             },
			section_story { "t of body for you",            status_type::READ_BODY             },
			section_story { "to enjoy plz thxx",            status_type::REJECTED_REQUEST_SIZE },
			section_story { "plz ignore this..",            status_type::REJECTED_REQUEST_SIZE },
		}};

		for ( const auto & section : sections )
		{
			const std::string s = std::get<0>(section);
			INFO("section: " << s);
			REQUIRE(std::get<1>(section) == parser.parse(s.c_str(), s.length()));
		}
	}
}
