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

#include <firebird/UdrCppEngine.h>


FB_UDR_BEGIN_PROCEDURE(gen_rows_message)
	FB_UDR_EXECUTE_MESSAGE_PROCEDURE(
		(FB_INTEGER, start)
		(FB_INTEGER, end)
	,
		(FB_INTEGER, result))
	{
		out->result = in->start - 1;
	}

	FB_UDR_FETCH_PROCEDURE
	{
		return out->result++ < in->end;
	}
FB_UDR_END_PROCEDURE


FB_UDR_BEGIN_PROCEDURE(gen_rows_message_5c)
	FB_UDR_EXECUTE_MESSAGE_PROCEDURE(
		(FB_INTEGER, start)
		(FB_INTEGER, end)
	,
		(FB_INTEGER, result1)
		(FB_INTEGER, result2)
		(FB_INTEGER, result3)
		(FB_INTEGER, result4)
		(FB_INTEGER, result5))
	{
		out->result1 = in->start - 1;
	}

	FB_UDR_FETCH_PROCEDURE
	{
		return (out->result2 = out->result3 = out->result4 = out->result5 = ++out->result1) <= in->end;
	}
FB_UDR_END_PROCEDURE


FB_UDR_BEGIN_PROCEDURE(copy_message)
	FB_UDR_EXECUTE_MESSAGE_PROCEDURE(
		(FB_INTEGER, count)
		(FB_VARCHAR(20), input)
	,
		(FB_VARCHAR(20), output))
	{
		memcpy(&out->output, &in->input, sizeof(in->input));
	}

	FB_UDR_FETCH_PROCEDURE
	{
		return in->count-- > 0;
	}
FB_UDR_END_PROCEDURE


FB_UDR_BEGIN_PROCEDURE(copy_message_5c)
	FB_UDR_EXECUTE_MESSAGE_PROCEDURE(
		(FB_INTEGER, count)
		(FB_VARCHAR(20), input)
	,
		(FB_VARCHAR(20), output1)
		(FB_VARCHAR(20), output2)
		(FB_VARCHAR(20), output3)
		(FB_VARCHAR(20), output4)
		(FB_VARCHAR(20), output5))
	{
		memcpy(&out->output1, &in->input, sizeof(in->input));
		memcpy(&out->output2, &in->input, sizeof(in->input));
		memcpy(&out->output3, &in->input, sizeof(in->input));
		memcpy(&out->output4, &in->input, sizeof(in->input));
		memcpy(&out->output5, &in->input, sizeof(in->input));
	}

	FB_UDR_FETCH_PROCEDURE
	{
		return in->count-- > 0;
	}
FB_UDR_END_PROCEDURE