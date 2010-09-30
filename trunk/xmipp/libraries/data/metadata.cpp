/***************************************************************************
 * 
 * Authors:      J.R. Bilbao-Castro (jrbcast@ace.ual.es)
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

#include "metadata.h"

//-----Constructors and related functions ------------
void MetaData::_clear(bool onlyData)
{
    if (onlyData)
    {
        myMDSql->deleteObjects();
    }
    else
    {
        path.clear();
        comment.clear();
        fastStringSearch.clear();
        fastStringSearchLabel = MDL_UNDEFINED;

        activeLabels.clear();
        ignoreLabels.clear();
        isColumnFormat = true;
        inFile = FileName::FileName();
        activeObjId = -1;//no active object
        delete iterObjectsId;
        iterObjectsId = NULL;
        myMDSql->clearMd();
    }
}//close clear

void MetaData::clear()
{
    //_clear(true);
    init();
}

void MetaData::init(const std::vector<MDLabel> *labelsVector)
{
    _clear();
    if (labelsVector != NULL)
        this->activeLabels = *labelsVector;
    //Create table in database
    myMDSql->createMd();
}//close init

void MetaData::copyInfo(const MetaData &md)
{
    if (this == &md) //not sense to copy same metadata
        return;
    this->setComment(md.getComment());
    this->setPath(md.getPath());
    this->isColumnFormat = md.isColumnFormat;
    this->inFile = md.inFile;
    this->fastStringSearchLabel = md.fastStringSearchLabel;
    this->activeLabels = md.activeLabels;
    this->ignoreLabels = md.ignoreLabels;

}//close copyInfo



void MetaData::copyMetadata(const MetaData &md)
{
    if (this == &md) //not sense to copy same metadata
        return;
    init(&(md.activeLabels));
    copyInfo(md);
    if (!md.activeLabels.empty())
    {
        md.myMDSql->copyObjects(this);
        firstObject(); //set first object as active
    }
    else
    {
        int n = md.size();
        for (int i = 0; i < n; i++)
            addObject();
    }
}

bool MetaData::_setValue(long int objId, const MDObject &mdValueIn)
{
    if (objId == -1)
    {
        if (activeObjId != -1)
            objId = activeObjId;
        else
        {
            REPORT_ERROR(ERR_MD_NOACTIVE, "setValue: please provide objId other than -1");
            exit(1);
        }
    }
    //add label if not exists, this is checked in addlabel
    addLabel(mdValueIn.label);
    //MDL::voidPtr2Value(label, valuePtr, mdValue);
    myMDSql->setObjectValue(objId, mdValueIn);
}

bool MetaData::_getValue(long int objId, MDObject &mdValueOut) const
{
    if (!containsLabel(mdValueOut.label))
        return false;

    if (objId == -1)
    {
        if (activeObjId != -1)
            objId = activeObjId;
        else
        {
            REPORT_ERROR(ERR_MD_NOACTIVE, "getValue: please provide objId other than -1");
            exit(1);
        }
    }
    //MDValue mdValue;
    return myMDSql->getObjectValue(objId, mdValueOut);
    //MDL::value2VoidPtr(label, mdValue, valuePtrOut);
}

bool MetaData::getRow(MDRow &row, long int objId)
{
    row.clear();
    MDObject * obj;
    for (std::vector<MDLabel>::const_iterator it = activeLabels.begin(); it != activeLabels.end(); ++it)
    {
        obj = new MDObject(*it);
        myMDSql->getObjectValue(objId, *obj);
        row.push_back(obj);
    }

    return true;
}

MetaData::MetaData()
{
    myMDSql = new MDSql(this);
    iterObjectsId = NULL;
    init(NULL);
}//close MetaData default Constructor

MetaData::MetaData(const std::vector<MDLabel> *labelsVector)
{
    myMDSql = new MDSql(this);
    iterObjectsId = NULL;
    init(labelsVector);
}//close MetaData default Constructor

MetaData::MetaData(const FileName &fileName, const std::vector<MDLabel> *labelsVector)
{
    myMDSql = new MDSql(this);
    iterObjectsId = NULL;
    init(labelsVector);
    //FIXME: what to do when labels vector is provided???
    read(fileName);
}//close MetaData from file Constructor

MetaData::MetaData(const MetaData &md)
{
    myMDSql = new MDSql(this);
    iterObjectsId = NULL;
    copyMetadata(md);
}//close MetaData copy Constructor

MetaData& MetaData::operator =(const MetaData &md)
{
    copyMetadata(md);
    return *this;
}//close metadata operator =

MetaData::~MetaData()
{
    _clear();
    delete myMDSql;
}//close MetaData Destructor

//-------- Getters and Setters ----------

bool MetaData::getColumnFormat() const
{
    return isColumnFormat;
}
/** Set to false for row format (parameter files)
 *  @ingroup GettersAndSetters
 *  set to true  for column format (this is the default) (docfiles)
 *
 */
void MetaData::setColumnFormat(bool column)
{
    isColumnFormat = column;
}
std::string MetaData::getPath()   const
{
    return path;
}

void MetaData::setPath(std::string newPath)
{
    const size_t length = 512;
    char _buffer[length];
    path = (newPath == "") ? std::string(getcwd(_buffer, length)) : newPath;
}

std::string MetaData::getComment() const
{
    return  comment;
}

void MetaData::setComment(const std::string &newComment)
{
    comment = newComment;
}

FileName MetaData::getFilename() const
{
    return inFile;
}

std::vector<MDLabel> MetaData::getActiveLabels() const
{
    return activeLabels;
}

std::vector<MDLabel>* MetaData::geActiveLabelsAddress() const
{
    return (std::vector<MDLabel>*) (&activeLabels);
}

long int  MetaData::getActiveObject() const
{
    return activeObjId;
}

int MetaData::MaxStringLength(const MDLabel thisLabel) const
{
    if (!containsLabel(thisLabel))
        return -1;

    return myMDSql->columnMaxLength(thisLabel);
}

bool MetaData::setValueFromStr(const MDLabel label, const std::string &value, long int objectId)
{
    addLabel(label);

    if (objectId == -1)
    {
        if (activeObjId != -1)
            objectId = activeObjId;
        else
        {
            REPORT_ERROR(ERR_MD_NOACTIVE, "setValue: please provide objId other than -1");
            exit(1);
        }
    }
    MDObject mdValue(label);
    mdValue.fromString(value);
    return myMDSql->setObjectValue(objectId, mdValue);
}

bool MetaData::getStrFromValue(const MDLabel label, std::string &strOut, long int objectId)
{
    MDObject mdValueOut(label);
    _getValue(objectId, mdValueOut);
    strOut = mdValueOut.toString();
}

bool MetaData::isEmpty() const
{
    return size() == 0;
}

long int MetaData::size() const
{
    std::vector<long int> objects;
    myMDSql->selectObjects(objects);

    return (long int)objects.size();
}

bool MetaData::containsLabel(const MDLabel label) const
{
    return vectorContainsLabel(activeLabels, label);
}

bool MetaData::addLabel(const MDLabel label)
{
    if (containsLabel(label))
        return false;
    activeLabels.push_back(label);
    myMDSql->addColumn(label);
    return true;
}

long int MetaData::addObject(long int objectId)
{
    activeObjId = myMDSql->addRow();
    return activeObjId;
}

void MetaData::importObject(const MetaData &md, const long int objId, bool doClear)
{
    md.myMDSql->copyObjects(this, new MDValueEQ(MDL_OBJID, objId));
}

void MetaData::importObjects(const MetaData &md, const std::vector<long int> &objectsToAdd, bool doClear)
{
    init(&(md.activeLabels));
    copyInfo(md);
    int size = objectsToAdd.size();
    for (int i = 0; i < size; i++)
        importObject(md, objectsToAdd[i]);
    firstObject();
}

void MetaData::importObjects(const MetaData &md, const MDQuery &query, bool doClear)
{
    if (doClear)
    {
        //Copy all structure and info from the other metadata
        init(&(md.activeLabels));
        copyInfo(md);
    }
    else
    {
        //If not clear, ensure that the have the same labels
        for (int i = 0; i < md.activeLabels.size(); i++)
            addLabel(md.activeLabels[i]);
    }
    md.myMDSql->copyObjects(this, &query);
    firstObject();
}

bool MetaData::removeObject(long int objectId)
{
    int removed = removeObjects(MDValueEQ(MDL_OBJID, objectId));
    firstObject();
    return (removed > 0);
}

void MetaData::removeObjects(const std::vector<long int> &toRemove)
{
    int size = toRemove.size();
    for (int i = 0; i < size; i++)
        removeObject(toRemove[i]);
    firstObject();
}

int MetaData::removeObjects(const MDQuery &query)
{
    int removed = myMDSql->deleteObjects(&query);
    firstObject(); //I prefer put active object to -1
    return removed;
}
int MetaData::removeObjects()
{
    int removed = myMDSql->deleteObjects();
    activeObjId = -1;
    return removed;
}

void MetaData::addIndex(MDLabel label)
{
    myMDSql->indexModify(label, true);
}

void MetaData::removeIndex(MDLabel label)
{
    myMDSql->indexModify(label, false);
}

long int MetaData::_iteratorBegin(const MDQuery *query)
{
    if (iterObjectsId == NULL)
        iterObjectsId = new std::vector<long int>;
    findObjects(*iterObjectsId);

    if (iterObjectsId->size() > 0)
    {
        iterIndex = 0;
        activeObjId = iterObjectsId->at(iterIndex);
    }
    else
    {
        activeObjId = iterIndex = -1;
    }
    return activeObjId;
}

long int MetaData::iteratorBegin()
{
    _iteratorBegin();
}

/**Same as previous but iterating over a subset of
 * objects
 */
long int MetaData::iteratorBegin(const MDQuery &query)
{
    _iteratorBegin(&query);
}

/** Check whether the iteration if finished */
bool MetaData::iteratorEnd() const
{
    return iterIndex == -1;
}

/** Move to next object on iteration
 * return nextObject id
 */
long int MetaData::iteratorNext()
{
    if (iterObjectsId != NULL && iterIndex < iterObjectsId->size() - 1)
    {
        //The end not reached yet
        iterIndex++;
        activeObjId = iterObjectsId->at(iterIndex);
    }
    else
    {
        activeObjId = iterIndex = -1;
    }
    return activeObjId;
}

//----------Iteration functions -------------------
long int MetaData::firstObject()
{
    //std::vector<long int> objects = MDSql::selectObjects(this, 1);
    //return (objects.size() > 0) ? objects[0] : -1;
    activeObjId = myMDSql->firstRow();
    return activeObjId;
}

long int MetaData::lastObject()
{
    activeObjId = myMDSql->lastRow();
    return activeObjId;
}

long int MetaData::nextObject()
{
    if (activeObjId == -1)
        REPORT_ERROR(ERR_MD_NOACTIVE, "nextObject: Couldn't perform this operation when 'activeObject' is -1");
    activeObjId = myMDSql->nextRow(activeObjId);
    return activeObjId;
}

long int MetaData::previousObject()
{
    if (activeObjId == -1)
        REPORT_ERROR(ERR_MD_NOACTIVE, "previousObject: Couldn't perform this operation when 'activeObject' is -1");
    activeObjId = myMDSql->previousRow(activeObjId);
    return activeObjId;
}

long int MetaData::goToObject(long int objectId)
{
    if (containsObject(objectId))
        activeObjId = objectId;
    else
        REPORT_ERROR(ERR_MD_NOOBJ, (std::string)"gotoObject: object with ID " + integerToString(objectId) + " doesn't exist in metadata");
    return activeObjId;

}

//-------------Search functions-------------------
void MetaData::findObjects(std::vector<long int> &objectsOut, const MDQuery &query)
{
    objectsOut.clear();
    myMDSql->selectObjects(objectsOut, &query);
}

void MetaData::findObjects(std::vector<long int> &objectsOut, int limit)
{
    objectsOut.clear();
    myMDSql->selectObjects(objectsOut, new MDQuery(limit));
}

int MetaData::countObjects(const MDQuery &query)
{
    std::vector<long int> objects;
    findObjects(objects, query);
    return objects.size();
}

bool MetaData::containsObject(long int objectId)
{
    return containsObject(MDValueEQ(MDL_OBJID, objectId));
}

bool MetaData::containsObject(const MDQuery &query)
{
    std::vector<long int> objects;
    findObjects(objects, query);
    return objects.size() > 0;
}

long int MetaData::gotoFirstObject(const MDQuery &query)
{
    std::vector<long int> objects;
    findObjects(objects, query);

    activeObjId = objects.size() == 1 ? objects[0] : -1;
    return activeObjId;
}

//--------------IO functions -----------------------

void MetaData::write(const FileName &outFile)
{
    // Open file
    std::ofstream ofs(outFile.data(), std::ios_base::out);
    write(ofs);
}

void MetaData::write(std::ostream &os)
{
    os << "; XMIPP_3 * " << (isColumnFormat ? "column" : "row")
    << "_format * " << path << std::endl //write wich type of format (column or row) and the path
    << "; " << comment << std::endl; //write md comment in the 2nd comment line of header

    if (isColumnFormat)
    {
        //write md columns in 3rd comment line of the header
        os << "; ";
        for (int i = 0; i < activeLabels.size(); i++)
        {
            if (activeLabels.at(i) != MDL_COMMENT)
            {
                os.width(10);
                os << MDL::label2Str(activeLabels.at(i)) << " ";
            }
        }
        os << std::endl;
        //Write data
        FOR_ALL_OBJECTS_IN_METADATA(*this)
        {
            for (int i = 0; i < activeLabels.size(); i++)
            {
                if (activeLabels[i] != MDL_COMMENT)
                {
                    MDObject mdValue(activeLabels[i]);
                    os.width(10);
                    myMDSql->getObjectValue(activeObjId, mdValue);
                    mdValue.toStream(os);
                    os << " ";
                }
            }
            os << std::endl;
        }
    }
    else //rowFormat
    {
        // Get first object. In this case (row format) there is a single object
        int objId = firstObject();

        if (objId != -1)
        {
            int maxWidth=20;
            for (int i = 0; i < activeLabels.size(); i++)
            {
                if (activeLabels.at(i) != MDL_COMMENT)
                {
                    int w=MDL::label2Str(activeLabels.at(i)).length();
                    if (w>maxWidth)
                        maxWidth=w;
                }
            }

            for (int i = 0; i < activeLabels.size(); i++)
            {
                if (activeLabels[i] != MDL_COMMENT)
                {
                    MDObject mdValue(activeLabels[i]);
                    os.width(maxWidth + 1);
                    os << MDL::label2Str(activeLabels.at(i)) << " ";
                    myMDSql->getObjectValue(objId, mdValue);
                    mdValue.toStream(os);
                    os << std::endl;
                }
            }
        }

    }
}//write

/** This function will read the posible columns from the file
 * and mark as MDL_UNDEFINED those who aren't valid labels
 * or those who appears in the IgnoreLabels vector
 * also set the activeLabels
 */
void MetaData::_readColumns(std::istream& is, MDRow & columnValues,
                            std::vector<MDLabel>* desiredLabels)
{
    std::string token;
    MDLabel label;

    while (is >> token)
        if (token.find('(') == std::string::npos)
        {
            //label is not reconized, the MDValue will be created
            //with MDL_UNDEFINED, wich will be ignored while reading data
            label = MDL::str2Label(token);
            if (desiredLabels != NULL && !vectorContainsLabel(*desiredLabels, label))
                label = MDL_UNDEFINED; //ignore if not present in desiredLabels
            columnValues.push_back(new MDObject(label));
            if (label != MDL_UNDEFINED)
                addLabel(label);
        }
}

/** This function will be used to parse the rows data
 * having read the columns labels before and setting wich are desired
 * the useCommentAsImage is for compatibility with old DocFile format
 * where the image were in comments
 */
void MetaData::_readRows(std::istream& is, MDRow & columnValues, bool useCommentAsImage)
{
    std::string line = "";
    while (!is.eof() && !is.fail())
    {
        //Move until the ';' or the first alphanumeric character
        while (is.peek() != ';' && isspace(is.peek()) && !is.eof())
            is.ignore(1);
        if (!is.eof())

            if (is.peek() == ';')//is a comment
            {
                is.ignore(1); //ignore the ';'
                getline(is, line);
                trim(line);
            }
            else if (!isspace(is.peek()))
            {
                addObject();
                if (line != "")
                {
                    if (!useCommentAsImage)
                        setValue(MDL_COMMENT, line);
                    else
                        setValue(MDL_IMAGE, line);
                }
                for (int i = 0; i < columnValues.size(); ++i)
                {
                    is >> *(columnValues[i]);
                    if (is.fail())
                    {
                        REPORT_ERROR(ERR_MD_BADLABEL, (std::string)"read: Error parsing data column, expecting " + MDL::label2Str(columnValues[i]->label));
                    }
                    else
                        if (columnValues[i]->label != MDL_UNDEFINED)
                            _setValue(activeObjId, *(columnValues[i]));
                }
            }

    }
}

/**This function will read the md data if is in row format */
void MetaData::_readRowFormat(std::istream& is)
{
    std::string line, token;
    MDLabel label;

    long int objectID = addObject();

    // Read data and fill structures accordingly
    while (getline(is, line, '\n'))
    {
        if (line[0] == '#' || line[0] == '\0' || line[0] == ';')
            continue;

        // Parse labels
        std::stringstream os(line);

        os >> token;
        label = MDL::str2Label(token);
        MDObject value(label);
        os >> value;
        if (label != MDL_UNDEFINED)
            _setValue(objectID, value);
    }
}

void MetaData::read(const FileName &filename, std::vector<MDLabel> *desiredLabels)
{
    std::ifstream is(filename.data(), std::ios_base::in);
    std::stringstream ss;
    std::string line, token;
    MDRow columnValues;


    getline(is, line); //get first line to identify the type of file

    if (is.fail())
    {
        REPORT_ERROR(ERR_IO_NOTEXIST, (std::string) "MetaData::read: File " + filename + " does not exists" );
    }

    _clear();
    myMDSql->createMd();
    isColumnFormat = true;
    bool useCommentAsImage = false;
    this->inFile = filename;

    is.seekg(0, std::ios::beg);//reset the stream position to the beginning to start parsing

    if (line.find("XMIPP_3 *") != std::string::npos)
    {
        //We have a new XMIPP MetaData format here, parse header
        is.ignore(256, '*') >> token; //Ignore all until first '*' and get md format in token
        isColumnFormat = token != "row_format";
        is.ignore(256, '*') >> token;
        if (token != ";") //there is path, need to ignore ';' of the next line
        {
            setPath(token);
            is.ignore(256, ';');
        }
        getline(is, line);
        setComment(line);
        if (isColumnFormat)
        {
            is.ignore(256, ';'); //ignore ';' to start parsing column labels
            getline(is, line);
            ss.str(line);
            //Read column labels
            _readColumns(ss, columnValues, desiredLabels);
        }
    }
    else if (line.find("Headerinfo columns:") != std::string::npos)
    {
        //This looks like an old DocFile, parse header
        std::cerr << "WARNING: ** You are using an old file format (DOCFILE) which is going "
        << "to be deprecated in next Xmipp release **" << std::endl;
        is.ignore(256, ':'); //ignore all until ':' to start parsing column labels
        getline(is, line);
        ss.str(line);
        columnValues.resize(2, new MDObject(MDL_UNDEFINED)); //start with 2 undefined to avoid 2 first columns of old format
        addLabel(MDL_IMAGE);
        _readColumns(ss, columnValues, desiredLabels);
        useCommentAsImage = true;
    }
    else
    {
        std::cerr << "WARNING: ** You are using an old file format (SELFILE) which is going "
        << "to be deprecated in next Xmipp release **" << std::endl;
        //I will assume that is an old SelFile, so only need to add two columns
        columnValues.push_back(new MDObject(MDL_IMAGE));//addLabel(MDL_IMAGE);
        columnValues.push_back(new MDObject(MDL_ENABLED));//addLabel(MDL_ENABLED);
    }

    if (isColumnFormat)
        _readRows(is, columnValues, useCommentAsImage);
    else
        _readRowFormat(is);
    firstObject();
}

void MetaData::merge(const FileName &fn)
{
    MetaData md;
    md.read(fn);
    unionAll(md);
}

void MetaData::aggregate(const MetaData &mdIn, AggregateOperation op,
                         MDLabel aggregateLabel, MDLabel operateLabel, MDLabel resultLabel)

{
    std::vector<MDLabel> labels(2);
    labels[0] = aggregateLabel;
    labels[1] = resultLabel;
    init(&labels);
    std::vector<AggregateOperation> ops(1);
    ops[0] = op;
    mdIn.myMDSql->aggregateMd(this, ops, operateLabel);
    firstObject();
}

void MetaData::aggregate(const MetaData &mdIn, const std::vector<AggregateOperation> &ops,
                         MDLabel operateLabel, const std::vector<MDLabel> &resultLabels)
{
    if (resultLabels.size() - ops.size() != 1)
        REPORT_ERROR(ERR_MD, "Labels vectors should contain one element more than operations");
    init(&resultLabels);
    mdIn.myMDSql->aggregateMd(this, ops, operateLabel);
    firstObject();
}


//-------------Set Operations ----------------------
void MetaData::_setOperates(const MetaData &mdIn, const MDLabel label, SetOperation operation)
{
    if (this == &mdIn) //not sense to operate on same metadata
        REPORT_ERROR(ERR_MD, "Couldn't perform this operation on input metadata");
    //Add labels to be sure are present
    for (int i = 0; i < mdIn.activeLabels.size(); i++)
        addLabel(mdIn.activeLabels[i]);

    mdIn.myMDSql->setOperate(this, label, operation);
    firstObject();
}

void MetaData::_setOperates(const MetaData &mdInLeft, const MetaData &mdInRight, const MDLabel label, SetOperation operation)
{
    if (this == &mdInLeft || this == &mdInRight) //not sense to operate on same metadata
        REPORT_ERROR(ERR_MD, "Couldn't perform this operation on input metadata");
    //Add labels to be sure are present
    for (int i = 0; i < mdInLeft.activeLabels.size(); i++)
        addLabel(mdInLeft.activeLabels[i]);
    for (int i = 0; i < mdInRight.activeLabels.size(); i++)
        addLabel(mdInRight.activeLabels[i]);

    myMDSql->setOperate(&mdInLeft, &mdInRight, label, operation);
    firstObject();
}

void MetaData::unionDistinct(const MetaData &mdIn, const MDLabel label)
{
    _setOperates(mdIn, label, UNION_DISTINCT);
}

void MetaData::unionAll(const MetaData &mdIn)
{
    _setOperates(mdIn, MDL_UNDEFINED, UNION);//label not needed for unionAll operation
}


void MetaData::intersection(const MetaData &mdIn, const MDLabel label)
{
    _setOperates(mdIn, label, INTERSECTION);
}
void MetaData::subtraction(const MetaData &mdIn, const MDLabel label)
{
    _setOperates(mdIn, label, SUBSTRACTION);
}

void MetaData::join(const MetaData &mdInLeft, const MetaData &mdInRight, const MDLabel label, JoinType type)
{
    _setOperates(mdInLeft, mdInRight, label, (SetOperation)type);
}

void MetaData::operate(const std::string &expression)
{
    if (!myMDSql->operate(expression))
        REPORT_ERROR(ERR_MD, "MetaData::operate: error doing operation");
}

void MetaData::randomize(MetaData &MDin)
{
    std::vector<long int> objects;
    MDin.myMDSql->selectObjects(objects);
    std::random_shuffle(objects.begin(), objects.end());
    importObjects(MDin, objects);
}

void MetaData::sort(MetaData &MDin, const MDLabel sortLabel)
{
    init(&(MDin.activeLabels));
    copyInfo(MDin);
    MDin.myMDSql->copyObjects(this, new MDQuery(-1, 0, sortLabel));
    firstObject();
}

void MetaData::split(int n, std::vector<MetaData> &results, const MDLabel sortLabel)
{
    long int mdSize = size();
    if (n > mdSize)
        REPORT_ERROR(ERR_MD, "MetaData::split: Couldn't split a metadata in more parts than its size");

    results.clear();
    results.resize(n);
    for (int i = 0; i < n; i++)
    {
        MetaData &md = results.at(i);
        md._selectSplitPart(*this, n, i, mdSize, sortLabel);
    }
}

void MetaData::_selectSplitPart(const MetaData &mdIn,
                                int n, int part, long int mdSize,
                                const MDLabel sortLabel)
{
    int first, last, n_images;
    n_images = divide_equally(mdSize, n, part, first, last);
    init(&(mdIn.activeLabels));
    copyInfo(mdIn);
    mdIn.myMDSql->copyObjects(this, new MDQuery(n_images, first, sortLabel));
    firstObject();
}

void MetaData::selectSplitPart(const MetaData &mdIn, int n, int part, const MDLabel sortLabel)
{
    long int mdSize = mdIn.size();
    if (n > mdSize)
        REPORT_ERROR(ERR_MD, "selectSplitPart: Couldn't split a metadata in more parts than its size");
    if (part < 0 || part >= n)
        REPORT_ERROR(ERR_MD, "selectSplitPart: 'part' should be between 0 and n-1");
    _selectSplitPart(mdIn, n, part, mdSize, sortLabel);

}

void MetaData::selectPart(const MetaData &mdIn, long int startPosition, long int numberOfObjects,
                          const MDLabel sortLabel)
{
    long int mdSize = mdIn.size();
    if (startPosition < 0 || startPosition >= mdSize)
        REPORT_ERROR(ERR_MD, "selectPart: 'startPosition' should be between 0 and size()-1");
    init(&(mdIn.activeLabels));
    copyInfo(mdIn);
    mdIn.myMDSql->copyObjects(this, new MDQuery(numberOfObjects, startPosition, sortLabel));
    firstObject();
}

