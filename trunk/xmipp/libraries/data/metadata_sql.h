/***************************************************************************
 *
 * Authors:     J.M. De la Rosa Trevin (jmdelarosa@cnb.csic.es)
 *
 * Unidad de  Bioinformatica of Centro Nacional de Biotecnologia , CSIC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 *
 *  All comments concerning this program package may be sent to the
 *  e-mail address 'xmipp@cnb.csic.es'
 ***************************************************************************/

#ifndef METADATASQL_H
#define METADATASQL_H

#include <iostream>
#include <map>
#include "strings.h"
#include <external/sqlite-3.6.23/sqlite3.h>
#include "metadata_label.h"

class MDSqlStaticInit;
class MDQuery;
class MetaData;
class MDCache;

/** @addtogroup MetaData
 * @{
 */

/** Posible Aggregation Operations in a MetaData */
enum AggregateOperation
{
    AGGR_COUNT, AGGR_MAX, AGGR_MIN, AGGR_SUM, AGGR_AVG
};

/** Posible Set Operations with MetaData */
enum SetOperation
{
    UNION, UNION_DISTINCT, INTERSECTION, SUBSTRACTION, INNER_JOIN, LEFT_JOIN, OUTER_JOIN
};

/** Enumeration of JOIN types for this operation */
enum JoinType
{
    INNER=INNER_JOIN, LEFT=LEFT_JOIN, OUTER=OUTER_JOIN
};

#include "metadata.h"

/** This class will manage SQL database interactions.
 * This class is designed to used inside a MetaData.
 */
class MDSql
{
public:
	static void dumpToFile(const FileName &fileName);

private:

    /** This will create the table to store the metada objects.
     * Will return false if the mdId table is already present.
     */
    bool createMd();

    /** This function will drop the entire table.
     * For use the metada again, a call to createMd() should be done.
     */
    bool clearMd();

    /**Add a new row and return the objId(rowId).
     */
    size_t addRow();

    /** Add a new column to a metadata.
     */
    bool addColumn(MDLabel column);

    /**Set the value of an object in an specified column.
     */
    bool setObjectValue(const int objId, const MDObject &value);

    /**Set the value of all objects in an specified column.
     */
    bool setObjectValue(const MDObject &value);

    /** Get the value of an object.
     */
    bool getObjectValue(const int objId, MDObject &value);

    /** This function will select some elements from table.
     * The 'limit' is the maximum number of object
     * returned, if is -1, all will be returned
     * Also a query could be specified for selecting objects
     * if no query is provided by default all are returned
     */
    void selectObjects(std::vector<size_t> &objectsOut, const MDQuery *queryPtr = NULL);

    /** This function will delete elements that match the query.
     * If not query is provided, all rows are deleted
     */
    size_t deleteObjects(const MDQuery *queryPtr = NULL);

    /** Coppy the objects from a metada to other.
     * return the number of objects copied
     * */
    size_t copyObjects(MDSql * sqlOut,
                         const MDQuery *queryPtr = NULL) const;
    size_t copyObjects(MetaData * mdPtrOut,
                         const MDQuery *queryPtr = NULL) const;

    /** This function performs aggregation operations.
     */
    void aggregateMd(MetaData *mdPtrOut,
                     const std::vector<AggregateOperation> &operations,
                     MDLabel operateLabel);

    /** This function performs aggregation operations.
        without grouping. (i.e. absolute maximum of a metadata column)
     */
    double aggregateSingleDouble(const AggregateOperation operation,
                                        MDLabel operateLabel);


    /** This function will be used to create o delete an index over a column.
     *Those indexes will improve searchs, but inserts will become expensives
     */
    void indexModify(const MDLabel label, bool create=true);

    /** Some iteration methods
     */
    size_t firstRow();
    size_t lastRow();
    size_t nextRow(size_t currentRow);
    size_t previousRow(size_t currentRow);

    int columnMaxLength(MDLabel column);

    /**Functions to implement set operations */
    void setOperate(MetaData *mdPtrOut, MDLabel column, SetOperation operation);
    void setOperate(const MetaData *mdInLeft, const MetaData *mdInRight, MDLabel column, SetOperation operation);
    /** Function to dump DB to file */
    bool operate(const String &expression);



    /** Constructor of MDSql
     * Now each MD should have an instance
     * of this class to interact with the DB
     */
    MDSql(MetaData *md);
    ~MDSql();

    static int table_counter;
    static sqlite3 *db;

    static MDSqlStaticInit initialization; //Just for initialization

    ///Just call this function once, at static initialization
    static bool sqlBegin();
    static bool sqlEnd();
    static bool sqlBeginTrans();
    static bool sqlCommitTrans();
    /** Return an unique id for each metadata
     * this function should be called once for each
     * metada and the id will be used for operations
     */
    int getUniqueId();

    bool dropTable();
    bool createTable(const std::vector<MDLabel> * labelsVector = NULL, bool withObjID=true);
    bool insertValues(double a, double b);
    void prepareStmt(const std::stringstream &ss, sqlite3_stmt *stmt);
    bool execSingleStmt(const std::stringstream &ss);
    bool execSingleStmt(sqlite3_stmt *&stmt, const std::stringstream *ss = NULL);
    size_t execSingleIntStmt(const std::stringstream &ss);
    double execSingleDoubleStmt(const std::stringstream &ss);

    String tableName(const int tableId) const;

    int bindValue(sqlite3_stmt *stmt, const int position, const MDObject &valueIn);
    int extractValue(sqlite3_stmt *stmt, const int position, MDObject &valueOut);

    static char *errmsg;
    static const char *zLeftover;
    static int rc;
    static sqlite3_stmt *stmt;

    ///Non-static attributes
    int tableId;
    MetaData *myMd;
    MDCache *myCache;

    friend class MDSqlStaticInit;
    friend class MetaData;
    friend class MDIterator;
}
;//close class MDSql

/** This is the base class for queries on MetaData.
 * It is abstract, so it can not be instanciated. Queries will be very
 * helpful for performing several tasks on MetaData like importing, searching
 * or removing objects.
 */
class MDQuery
{
public:
    int limit; ///< If distint of -1 the results will be limited to this value
    int offset; ///< If distint of 0, offset elements will be discarded
    MDLabel orderLabel; ///< Label to wich apply sort of the results

    /** Constructor. */
    MDQuery(int limit = -1, int offset = 0, MDLabel orderLabel = MDL_OBJID)
    {
        this->limit = limit;
        this->offset = offset;
        this->orderLabel = orderLabel;
    }

    /** Return the ORDER BY string to be used in SQL query */
    String orderByString() const
    {
        return (String)" ORDER BY " + MDL::label2Str(orderLabel);
    }

    /** Return the LIMIT string to be used in SQL */
    String limitString() const
    {
        std::stringstream ss;
        if (limit != -1)
            ss << " LIMIT " << limit << " ";
        if (offset > 0)
            ss << "OFFSET " << offset << " ";
        return ss.str();
    }

    /** Return the WHERE string to be used in SQL query */
    String whereString() const
    {
        String queryString = this->queryStringFunc();
        return (queryString == " ") ? " " : " WHERE " + queryString + " ";
    }

    /** Return the query string, should be overrided in subclasses */
    virtual String queryStringFunc() const
    {
        return " ";
    }
}
;//End of class MDQuery

/** @} */

/** Enumeration of all posible relational queries operations */
enum RelationalOp
{
    EQ, ///< Equal
    NE, ///< Not equal
    GT, ///< Greater than
    LT, ///< Less than
    GE, ///< Greater equal
    LE ///< Less equal
};

/** Subclass of MDQuery and base for all relational queries.
 * This kind of query can be used to compare some LABEL values
 * in a column of the MetaData with an specified VALUE.
 * @see RelationalOp for possible relational operations.
 */
class MDValueRelational: public MDQuery
{
    MDObject *value;
    RelationalOp op;
public:


    MDValueRelational()
    {
        op = EQ;
        value = NULL;
    }

    template <class T>
    MDValueRelational(MDLabel label, const T &value, RelationalOp op, int limit = -1, int offset = 0, MDLabel orderLabel = MDL_OBJID):MDQuery(limit, offset, orderLabel)
    {
        this->op = op;
        this->value = new MDObject(label, value);
    }

    MDValueRelational(const MDObject &value, RelationalOp op, int limit = -1, int offset = 0, MDLabel orderLabel = MDL_OBJID):MDQuery(limit, offset, orderLabel)
    {
        this->op = op;
        this->value = new MDObject(value);
    }

    ~MDValueRelational()
    {
        delete this->value;
    }

    String opString() const
    {
        switch (op)
        {
        case EQ:
            return "=";
        case NE:
            return "!=";
        case GT:
            return ">";
        case LT:
            return "<";
        case GE:
            return ">=";
        case LE:
            return "<=";
        }
    }

    virtual String queryStringFunc() const
    {
        return (value == NULL) ? " " : MDL::label2Str(value->label) + opString() + value->toString(false, true);
    }

    template <class T>
    void setValue(T &value)
    {
        if (this->value != NULL)
            delete this->value;
        this->value = new MDObject(this->value->label, value);
    }
}
;//end of class MDValueRelational

/** Query if MetaData column values are equal to some specific value.
 * @code
 *  ///Remove all images that are disabled
 *  MetaData md1, md2;
 *  md1.removeObjects(MDValueEQ(MDL_ENABLED, -1));
 *  ///Import objects from md2 to md1 wich rot angle is 0.
 *  md1.importObjects(md2, MDValueEQ(MDL_ANGLE_ROT, 0.));
 *  @endcode
 */
class MDValueEQ: public MDValueRelational
{
public:
    MDValueEQ()
    {}
    template <class T>
    MDValueEQ(MDLabel label, const T &value, int limit = -1, int offset = 0, MDLabel orderLabel = MDL_OBJID)
            :MDValueRelational(label, value, EQ, limit, offset, orderLabel)
    {}
}
;//end of class MDValueEQ

/** Query if MetaData column values are distint than some specific value.
 * @see MDValueEQ for examples of use.
 */
class MDValueNE: public MDValueRelational
{
public:
    MDValueNE()
    {}
    template <class T>
    MDValueNE(MDLabel label, const T &value, int limit = -1, int offset = 0, MDLabel orderLabel = MDL_OBJID)
            :MDValueRelational(label, value, NE, limit, offset, orderLabel)
    {}
}
;//end of class MDValueNE

/** Query if MetaData column values are greater equal than some specific value.
 * @see MDValueEQ for examples of use.
 */
class MDValueGE: public MDValueRelational
{
public:
    MDValueGE()
    {}
    template <class T>
    MDValueGE(MDLabel label, const T &valueMin, int limit = -1,int offset = 0, MDLabel orderLabel = MDL_OBJID)
            :MDValueRelational(label, valueMin, GE, limit, offset, orderLabel)
    {}
}
;//end of class MDValueGE

/** Query if MetaData column values are greater than some specific value.
 * @see MDValueEQ for examples of use.
 */
class MDValueGT: public MDValueRelational
{
public:
    MDValueGT()
    {}
    template <class T>
    MDValueGT(MDLabel label, const T &valueMin, int limit = -1,int offset = 0, MDLabel orderLabel = MDL_OBJID)
            :MDValueRelational(label, valueMin, GT, limit, offset, orderLabel)
    {}
}
;//end of class MDValueGT

/** Query if MetaData column values are less or equal than some specific value.
 * @see MDValueEQ for examples of use.
 */
class MDValueLE: public MDValueRelational
{
public:
    MDValueLE()
    {}
    template <class T>
    MDValueLE(MDLabel label, const T &valueMax, int limit = -1,int offset = 0, MDLabel orderLabel = MDL_OBJID)
            :MDValueRelational(label, valueMax, LE, limit, offset, orderLabel)
    {}
}
;//end of class MDValueLE

/** Query if MetaData column values are less than some specific value.
 * @see MDValueEQ for examples of use.
 */
class MDValueLT: public MDValueRelational
{
public:
    MDValueLT()
    {}
    template <class T>
    MDValueLT(MDLabel label, const T &valueMax, int limit = -1,int offset = 0, MDLabel orderLabel = MDL_OBJID)
            :MDValueRelational(label, valueMax, LT, limit, offset, orderLabel)
    {}
}
;//end of class MDValueLT

/**This subclass of Query will test if a label have a value within a minimum and maximum.
 * @code
 *  //Remove all images with rotational angle between 100 and 200
 *  MetaData md;
 *  md.removeObjects(MDValueRange(MDL_ANGLE_ROT, 100., 200.));
 *  @endcode
 */
class MDValueRange: public MDQuery
{
    MDValueRelational *query1, *query2;
public:
    MDValueRange()
    {
        query1=query2=NULL;
    }
    template <class T>
    MDValueRange(MDLabel label, const T &valueMin, const T &valueMax,
                 int limit = -1, int offset = 0, MDLabel orderLabel = MDL_OBJID):MDQuery(limit, offset, orderLabel)
    {
        query1 = new MDValueRelational(label, valueMin, GE);
        query2 = new MDValueRelational(label, valueMax, LE);
    }

    MDValueRange(const MDObject &o1, const MDObject &o2,
        int limit = -1, int offset = 0, MDLabel orderLabel = MDL_OBJID):MDQuery(limit, offset, orderLabel)
    {
        if (o1.label != o2.label)
          REPORT_ERROR(ERR_VALUE_INCORRECT, "Labels should be the same");
        query1 = new MDValueRelational(o1, GE);
        query2 = new MDValueRelational(o2, LE);

    }
    virtual String queryStringFunc() const
    {
        std::stringstream ss;
        ss << "(" << query1->queryStringFunc() << " AND " << query2->queryStringFunc() << ")";
        return ss.str();
    }

    ~MDValueRange()
    {
        delete query1;
        delete query2;
    }
}
;//end of class MDValueRange

/**This subclass of Query will select those entries that satisfy an expression.
 * @code
 *  //Remove all images with rotational angle between 100 and 200
 *  MetaData md;
 *  md.removeObjects(MDExpression("angleRot > 100 AND angleRot < 200"));
 *  @endcode
 */
class MDExpression: public MDQuery
{
	String sExpression;
public:
	MDExpression()
    {
		sExpression = " 1=1 ";
    }
	MDExpression(String _sExpression,
			     int limit = -1,
			     int offset = 0,
			     MDLabel orderLabel = MDL_OBJID):MDQuery(limit, offset, orderLabel)
    {
		sExpression=_sExpression;
    }

    virtual String queryStringFunc() const
    {
        return sExpression;
    }

}
;//end of class MDExpression

/** Query several conditions using AND and OR.
 * This kind of query if usefull if you want to check
 * two conditions at the same time, for example, import
 * all images that are enabled and have rotational angle greater than 100.
 * @code
 *  MetaData md1, md2;
 *  MDValueEQ eq(MDL_ENABLED, 1);
 *  MDValueGT gt(MDL_ANGLE_ROT, 100.);
 *  MDMultiQuery multi;
 *  //The first query added has the same effect doing with AND or OR
 *  multi.addAndQuery(eq);
 *  multi.addAndQuery(gt);
 *
 *  md1.importObjects(md2, multi);
 * @endcode
 */
class MDMultiQuery: public MDQuery
{
private:
    std::vector<const MDQuery*> queries;
    std::vector<String> operations;

public:

    MDMultiQuery(int limit = -1, int offset = 0, MDLabel orderLabel = MDL_OBJID):MDQuery(limit, offset, orderLabel)
    {
        clear();
    }
    void addAndQuery(MDQuery &query)
    {
        queries.push_back(&query);
        operations.push_back("AND");
    }
    void addOrQuery(MDQuery &query)
    {
        queries.push_back(&query);
        operations.push_back("OR");
    }

    void clear()
    {
        queries.clear();
        operations.clear();
    }

    virtual String queryStringFunc() const
    {
        if (queries.size() > 0)
        {
            std::stringstream ss;
            ss << "(" << queries[0]->queryStringFunc() << ") ";
            for (int i = 1; i < queries.size(); i++)
                ss << operations[i] << " (" << queries[i]->queryStringFunc() << ") ";

            return ss.str();
        }
        return " ";
    }

}
;//end of class MDMultiQuery

/** Class to store some cached sql statements.
 */
class MDCache
{
public:
    sqlite3_stmt *iterStmt;
    std::map<MDLabel, sqlite3_stmt*> getValueCache;
    std::map<MDLabel, sqlite3_stmt*> setValueCache;
    sqlite3_stmt *addRowStmt;

    MDCache();
    ~MDCache();
    void clear();
};

/** Just to work as static constructor for initialize database.
 */
class MDSqlStaticInit
{
private:
    MDSqlStaticInit()
    {
        MDSql::sqlBegin();
    }//close constructor

    ~MDSqlStaticInit()
    {
        MDSql::sqlEnd();
    }//close destructor

    friend class MDSql;
}
;//close class MDSqlStaticInit


template<class T>
MDValueEQ MDValueEqualSwig(MDLabel label, const T &value)
{
    return MDValueEQ(label, value);
}

template<class T>
MDValueRange MDValueRangeSwig(MDLabel label, const T &valueMin, const T &valueMax)
{
    return MDValueRange(label, valueMin, valueMax);
}

template<class T>
MDValueGE MDValueAboveSwig(MDLabel label, const T &valueMin)
{
    return MDValueGE(label, valueMin);
}

template<class T>
MDValueLE MDValueBelowSwig(MDLabel label, const T &valueMax)
{
    return MDValueLE(label, valueMax);
}
#endif
