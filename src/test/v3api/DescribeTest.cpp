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
 * Portions created by the Initial Developer are Copyright (C) 2011 the Initial Developer.
 * All Rights Reserved.
 *
 * Contributor(s):
 *
 */

#include "V3Util.h"
#include <string>
#include <boost/test/unit_test.hpp>

using std::string;
using namespace V3Util;
using namespace Firebird;

//------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(v3api)


BOOST_AUTO_TEST_CASE(describe)
{
	const unsigned char* testDpb[2] = {NULL, FbTest::ASCII_DPB};
	unsigned testDpbLength[2] = {0, sizeof(FbTest::ASCII_DPB)};
	unsigned testSubType[2] = {3, 2};
	unsigned testLength[2] = {31 * 3, 31};

	for (unsigned test = 0; test < 2; ++test)
	{
		const string location = FbTest::getLocation("describe.fdb");

		CheckStatusWrapper statusWrapper(master->getStatus());
		CheckStatusWrapper* status = &statusWrapper;

		IAttachment* attachment = dispatcher->createDatabase(status, location.c_str(),
			testDpbLength[test], testDpb[test]);
		BOOST_CHECK(checkStatus(status));
		BOOST_REQUIRE(attachment);

		ITransaction* transaction = attachment->startTransaction(status, 0, NULL);
		BOOST_CHECK(checkStatus(status));
		BOOST_REQUIRE(transaction);

		unsigned major;
		getEngineVersion(attachment, &major);

		// Test with and without metadata prefetch.
		for (unsigned i = 0; i < 2; ++i)
		{
			bool prefetch = (i == 0);

			IStatement* stmt = attachment->prepare(status, transaction, 0,
				"select rdb$relation_id relid, rdb$character_set_name csname"
				"  from rdb$database"
				"  where rdb$relation_id < ?",
				FbTest::DIALECT, (prefetch ? IStatement::PREPARE_PREFETCH_ALL : 0));
			BOOST_CHECK(checkStatus(status));
			BOOST_REQUIRE(stmt);

			unsigned type = stmt->getType(status);
			BOOST_CHECK(checkStatus(status));

			string legacyPlan = stmt->getPlan(status, false);
			BOOST_CHECK(checkStatus(status));

			string detailedPlan = major >= 3 ? stmt->getPlan(status, true) : "";
			BOOST_CHECK(checkStatus(status));

			IMessageMetadata* inputParams = stmt->getInputMetadata(status);
			BOOST_CHECK(checkStatus(status));
			BOOST_REQUIRE(inputParams);

			IMessageMetadata* outputParams = stmt->getOutputMetadata(status);
			BOOST_CHECK(checkStatus(status));
			BOOST_REQUIRE(outputParams);

			BOOST_CHECK_EQUAL(type, isc_info_sql_stmt_select);

			BOOST_CHECK_EQUAL(legacyPlan,
				"\nPLAN (RDB$DATABASE NATURAL)");

			if (major >= 3)
			{
				BOOST_CHECK_EQUAL(detailedPlan,
					"\nSelect Expression\n"
					"    -> Filter\n"
					"        -> Table \"RDB$DATABASE\" Full Scan");
			}

			struct FieldInfo
			{
				string field;
				string relation;
				string owner;
				string alias;
				unsigned type;
				bool nullable;
				unsigned charSet;
				unsigned length;
				unsigned scale;

				static void test(CheckStatusWrapper* status, IMessageMetadata* params,
					unsigned count, FieldInfo* fieldInfo)
				{
					BOOST_CHECK_EQUAL(params->getCount(status), count);
					BOOST_CHECK(checkStatus(status));

					for (unsigned i = 0; i < count; ++i)
					{
						BOOST_CHECK_EQUAL(params->getField(status, i), fieldInfo[i].field);
						BOOST_CHECK(checkStatus(status));

						BOOST_CHECK_EQUAL(params->getRelation(status, i), fieldInfo[i].relation);
						BOOST_CHECK(checkStatus(status));

						BOOST_CHECK_EQUAL(params->getOwner(status, i), fieldInfo[i].owner);
						BOOST_CHECK(checkStatus(status));

						BOOST_CHECK_EQUAL(params->getAlias(status, i), fieldInfo[i].alias);
						BOOST_CHECK(checkStatus(status));

						BOOST_CHECK_EQUAL(params->getType(status, i), fieldInfo[i].type);
						BOOST_CHECK(checkStatus(status));

						BOOST_CHECK_EQUAL(params->isNullable(status, i), fieldInfo[i].nullable);
						BOOST_CHECK(checkStatus(status));

						BOOST_CHECK_EQUAL(params->getCharSet(status, i), fieldInfo[i].charSet);
						BOOST_CHECK(checkStatus(status));

						BOOST_CHECK_EQUAL(params->getLength(status, i), fieldInfo[i].length);
						BOOST_CHECK(checkStatus(status));

						BOOST_CHECK_EQUAL(params->getScale(status, i), fieldInfo[i].scale);
						BOOST_CHECK(checkStatus(status));
					}
				}
			};

			FieldInfo inputInfo[] = {
				{"", "", "", "", SQL_SHORT, true, 0, sizeof(ISC_SHORT), 0}
			};

			FieldInfo::test(status, inputParams, sizeof(inputInfo) / sizeof(inputInfo[0]), inputInfo);

			FieldInfo outputInfo[] = {
				{"RDB$RELATION_ID",        "RDB$DATABASE", "SYSDBA", "RELID",  SQL_SHORT, true,
					0,                 sizeof(ISC_SHORT),   0},
				{"RDB$CHARACTER_SET_NAME", "RDB$DATABASE", "SYSDBA", "CSNAME", SQL_TEXT,  true,
					testSubType[test], testLength[test], 0}
			};

			FieldInfo::test(status, outputParams, sizeof(outputInfo) / sizeof(outputInfo[0]), outputInfo);

			outputParams->release();
			inputParams->release();

			stmt->free(status);
			BOOST_CHECK(checkStatus(status));
		}

		transaction->commit(status);
		BOOST_CHECK(checkStatus(status));

		attachment->dropDatabase(status);
		BOOST_CHECK(checkStatus(status));

		status->dispose();
	}
}


BOOST_AUTO_TEST_SUITE_END()
