//
// Library: Net Responsibility
// Package: Core
// Module:  Blacklist
//
// This file is part of Net Responsibility.
//
// Net Responsibility is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Net Responsibility is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Net Responsibility.  If not, see <http://www.gnu.org/licenses/>.
//
// <Blacklist> provides some structs for storing the blacklists and matches



#ifndef BLACKLIST_H_INCLUDED
#define BLACKLIST_H_INCLUDED

#include <vector>

#include "Poco/RegularExpression.h"
#include "Poco/SharedPtr.h"
#include "Poco/Timestamp.h"
#include "Poco/Data/Binding.h"
#include "Poco/Data/Extraction.h"
#include "Poco/Data/Limit.h"
#include "Poco/Data/SessionFactory.h"
#include "Poco/Data/Connector.h"
#include "Poco/Data/SQLite/Connector.h"

using Poco::Timestamp;
using namespace Poco::Data;
using Poco::SharedPtr;
using Poco::RegularExpression;
using namespace std;

struct BlacklistKeyword
	/// This is a small struct to hold each keyword, as a string, compiled as
	/// separate regular expressions and the adherent strength.
{
	int strength;
		/// The strength of this keyword. 100 is default.

	string asString;
		/// The keyword displayed as a string.

	string category;

	vector< SharedPtr<RegularExpression> > re;
		/// The keyword splitted and compiled as a regular expression.
};



struct BlacklistCategory
	/// This struct holds each specific category.
{
	string name;
		/// The name of the category.

	vector<BlacklistKeyword> keyword;	// Change to keywords since its more than one
		/// All the keywords.
};



struct BlacklistMatch
	/// If the filter catches a match, in other words a suspicious URL, it's
	/// stored inside a BlacklistMatch. Here we have all the important
	/// information related to the match.
{
	string boldUrl;
		/// The URL with every suspicious word in bold.

	string abbrUrl;
		/// The same as boldUrl, but abbreviated to be easier to read and save
		/// some space.

	int strength;
		/// The total strength of the match. This is displayed in the reports
		/// if this strength is lower than reportStrengthThreshold.

	bool whitelist;
		/// This bool is set to true if it is a whitelist match. In that case it
		/// will show up in the whitelist section of the report.

	vector<BlacklistKeyword> keyword;	// Change to keywords, more than one.
		/// The keywords that are found in the URL.
};



typedef vector<BlacklistCategory> Blacklist;
	/// Blacklist is the top-level container for all the blacklists.
	// Change this typedef to:
	// typedef map<string, vector<BlacklistKeyword> > Blacklist;
	// This way we can skip BlacklistCategory.



struct Extension {
	/// The strength of the BlacklistMatch will also take the type of the URL in
	/// consideration. If it's an image or video it will be much stronger than
	/// if it is a script. This class holds different extensions and their
	/// associated types.
	public:
		int strength;
			/// The strength of this specific extension.

		string group;
			/// What kind of type it is. Image/Video etc.

		SharedPtr<RegularExpression> re;
			/// The regular expression to compare to the extension of the URL.
};



typedef vector<Extension> Extensions;


namespace Poco {
namespace Data {

template <>
class TypeHandler<class BlacklistMatch>
{
public:
	static size_t size()
	{
		return 4; // we handle four columns of the Table!
	}

	static void bind(size_t pos, const BlacklistMatch& obj,
			AbstractBinder::Ptr pBinder, AbstractBinder::Direction dir)
	{
		poco_assert_dbg (!pBinder.isNull());
		TypeHandler<string>::bind(pos++, obj.boldUrl, pBinder, dir);
		TypeHandler<string>::bind(pos++, obj.abbrUrl, pBinder, dir);
		TypeHandler<int>::bind(pos++, obj.strength, pBinder, dir);
		TypeHandler<int>::bind(pos++, (int)obj.whitelist, pBinder, dir);
	}

	static void prepare(size_t pos, const BlacklistMatch& obj,
			AbstractPreparator::Ptr pPrepare)
	{
		poco_assert_dbg (!pPrepare.isNull());
		TypeHandler<string>::prepare(pos++, obj.boldUrl, pPrepare);
		TypeHandler<string>::prepare(pos++, obj.abbrUrl, pPrepare);
		TypeHandler<int>::prepare(pos++, obj.strength, pPrepare);
		TypeHandler<int>::prepare(pos++, (int)obj.whitelist, pPrepare);
	}

	static void extract(size_t pos, BlacklistMatch& obj,
			const BlacklistMatch& defVal, AbstractExtractor* pExt)
		/// obj will contain the result, defVal contains values we should use when one column is NULL
	{
		poco_assert_dbg (pExt != 0);
		int w;
		TypeHandler<string>::extract(pos++, obj.boldUrl, defVal.boldUrl, pExt);
		TypeHandler<string>::extract(pos++, obj.abbrUrl, defVal.abbrUrl, pExt);
		TypeHandler<int>::extract(pos++, obj.strength, defVal.strength, pExt);
		TypeHandler<int>::extract(pos++, w, defVal.whitelist, pExt);
		obj.whitelist = (w != 0);
	}
};


template <>
class TypeHandler<class BlacklistKeyword>
{
public:
	static size_t size()
	{
		return 3; // we handle four columns of the Table!
	}

	static void bind(size_t pos, const BlacklistKeyword& obj,
			AbstractBinder::Ptr pBinder, AbstractBinder::Direction dir)
	{
		poco_assert_dbg (!pBinder.isNull());
		TypeHandler<string>::bind(pos++, obj.asString, pBinder, dir);
		TypeHandler<string>::bind(pos++, obj.category, pBinder, dir);
		TypeHandler<int>::bind(pos++, obj.strength, pBinder, dir);
	}

	static void prepare(size_t pos, const BlacklistKeyword& obj,
			AbstractPreparator::Ptr pPrepare)
	{
		poco_assert_dbg (!pPrepare.isNull());
		TypeHandler<string>::prepare(pos++, obj.asString, pPrepare);
		TypeHandler<string>::prepare(pos++, obj.category, pPrepare);
		TypeHandler<int>::prepare(pos++, obj.strength, pPrepare);
	}

	static void extract(size_t pos, BlacklistKeyword& obj,
			const BlacklistKeyword& defVal, AbstractExtractor* pExt)
		/// obj will contain the result, defVal contains values we should use when one column is NULL
	{
		poco_assert_dbg (pExt != 0);
		TypeHandler<string>::extract(pos++, obj.asString, defVal.asString, pExt);
		TypeHandler<string>::extract(pos++, obj.category, defVal.category, pExt);
		TypeHandler<int>::extract(pos++, obj.strength, defVal.strength, pExt);
	}
};

} } // namespace Poco::Data


#endif // BLACKLIST_H_INCLUDED
