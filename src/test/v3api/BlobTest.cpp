/*
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is Adriano dos Santos Fernandes.
 * Portions created by the Initial Developer are Copyright (C) 2013 the Initial Developer.
 * All Rights Reserved.
 *
 * Contributor(s):
 *
 */

#include "V3Util.h"
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>

using namespace V3Util;
using namespace Firebird;
using std::string;
using boost::lexical_cast;

//------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(v3api)


BOOST_AUTO_TEST_CASE(blob)
{
	const string location = FbTest::getLocation("blob.fdb");

	IStatus* status = master->getStatus();

	IAttachment* attachment = dispatcher->createDatabase(status, location.c_str(), 0, NULL);
	BOOST_CHECK(status->isSuccess());
	BOOST_REQUIRE(attachment);

	ITransaction* transaction = attachment->startTransaction(status, 0, NULL);
	BOOST_CHECK(status->isSuccess());
	BOOST_REQUIRE(transaction);

	// Create test table.
	{
		attachment->execute(status, transaction, 0,
			"create table test (b1 blob)", FbTest::DIALECT, NULL, NULL, NULL, NULL);
		BOOST_CHECK(status->isSuccess());

		transaction->commitRetaining(status);
		BOOST_CHECK(status->isSuccess());
	}

	{
		const char* const SEGMENTS[] = {
			"1234567890",
			"",
			"abcdefghij",
			NULL
		};

		IStatement* stmt = attachment->prepare(status, transaction, 0,
			"insert into test (b1) values (?)", FbTest::DIALECT, 0);
		BOOST_CHECK(status->isSuccess());
		BOOST_REQUIRE(stmt);

		{
			FB_MESSAGE(InputType,
				(FB_BLOB, b1)
			) input(master);

			input.clear();
			IBlob* blob = attachment->createBlob(status, transaction, &input->b1);
			BOOST_CHECK(status->isSuccess());

			for (const char* const* segment = SEGMENTS; *segment; ++segment)
			{
				blob->putSegment(status, strlen(*segment), *segment);
				BOOST_CHECK(status->isSuccess());
			}

			blob->close(status);
			BOOST_CHECK(status->isSuccess());

			stmt->execute(status, transaction, input.getMetadata(), input.getData(), NULL, NULL);
			BOOST_CHECK(status->isSuccess());

			stmt->free(status);
			BOOST_CHECK(status->isSuccess());
		}

		{
			stmt = attachment->prepare(status, transaction, 0,
				"select b1 from test", FbTest::DIALECT, 0);
			BOOST_CHECK(status->isSuccess());

			FB_MESSAGE(OutputType,
				(FB_BLOB, b1)
			) output(master);

			IResultSet* rs = stmt->openCursor(status, transaction, NULL, NULL,
				output.getMetadata());
			BOOST_CHECK(status->isSuccess());

			BOOST_CHECK(rs->fetchNext(status, output.getData()) != 100);

			IBlob* blob = attachment->openBlob(status, transaction, &output->b1);
			BOOST_CHECK(status->isSuccess());

			for (const char* const* segment = SEGMENTS;; ++segment)
			{
				char data[30];
				unsigned len = blob->getSegment(status, sizeof(data) - 1, data);
				data[len] = '\0';

				if (*segment)
				{
					BOOST_CHECK(status->isSuccess());
					BOOST_CHECK(strcmp(data, *segment) == 0);
				}
				else
				{
					BOOST_CHECK(len == 0 && status->get()[1] == isc_segstr_eof);
					break;
				}
			}

			blob->close(status);
			BOOST_CHECK(status->isSuccess());

			rs->close(status);
			BOOST_CHECK(status->isSuccess());

			stmt->free(status);
			BOOST_CHECK(status->isSuccess());
		}
	}

	transaction->commit(status);
	BOOST_CHECK(status->isSuccess());

	attachment->dropDatabase(status);
	BOOST_CHECK(status->isSuccess());

	status->dispose();
}


BOOST_AUTO_TEST_SUITE_END()