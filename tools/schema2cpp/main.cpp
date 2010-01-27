#include <QtCore/QCoreApplication>
#include <QTextStream>
#include <QDomDocument>
#include <QDomNodeList>
#include <QFile>
#include <QDir>
#include <QSharedPointer>
#include <QHash>
#include <QSet>
#include <iostream>


const QString INDENT = "    ";
const QString INDENT2 = INDENT + INDENT;
const QString INDENT3 = INDENT2 + INDENT;

const QString CLASS_PREFIX = "Scenario";


QString capitalizeFirst(const QString& s)
{
    if (s.isEmpty())
    {
        return s;
    }
    else
    {
        return s.at(0).toUpper() + s.right(s.length() - 1);
    }
}

QString lowercaseFirst(const QString& s)
{
    if (s.isEmpty())
    {
        return s;
    }
    else
    {
        return s.at(0).toLower() + s.right(s.length() - 1);
    }
}

QString quoteString(const QString& s)
{
    return "\"" + s + "\"";
}

QString stripNamespace(const QString& s)
{
    int position = s.indexOf(":");
    if (s > 0)
    {
        return s.right(s.length() - position - 1);
    }
    else
    {
        return s;
    }
}


class SchemaType
{
public:
    enum Type
    {
        Boolean,
        Double,
        Integer,
        String,
        DateTime,
        DoubleList,
        IntegerList,
        StringList,
        Custom
    };

    SchemaType(Type type) :
        m_type(type)
    {
    }

    SchemaType(QString customType) :
        m_type(Custom),
        m_customType(customType)
    {
    }

    SchemaType(const SchemaType& other) :
        m_type(other.m_type),
        m_customType(other.m_customType)
    {
    }

    SchemaType operator=(const SchemaType& other)
    {
        m_type = other.m_type;
        m_customType = other.m_customType;

        return *this;
    }

    Type type() const
    {
        return m_type;
    }

    bool isCustom() const
    {
        return type() == Custom;
    }

    bool isArrayType() const
    {
        switch (m_type)
        {
        case DoubleList:
        case StringList:
        case IntegerList:
            return true;
        default:
            return false;
        }
    }

    QString ctype() const
    {
        switch (m_type)
        {
        case Boolean:
            return "bool";
        case Double:
            return "double";
        case Integer:
            return "int";
        case DateTime:
            return "QDateTime";
        case String:
            return "QString";
        case DoubleList:
            return "QList<double>";
        case StringList:
            return "QList<QString>";
        case IntegerList:
            return "QList<int>";
        case Custom:
            return CLASS_PREFIX + m_customType;
        default:
            Q_ASSERT(0);
            return "BAD";
        }
    }

    QString memberVariableType() const
    {
        if (isCustom())
        {
            return "QSharedPointer<" + ctype() + ">";
        }
        else
        {
            return ctype();
        }
    }

    QString memberVariableVectorType() const
    {
        if (isCustom())
        {
            return "QList<QSharedPointer<" + ctype() + "> >";
        }
        else
        {
            return "QList<" + ctype() + ">";
        }
    }

    QString parsingFunction() const
    {
        switch (m_type)
        {
        case Boolean:
            return "parseBoolean";
        case Double:
            return "parseDouble";
        case Integer:
            return "parseInt";
        case DateTime:
            return "parseDateTime";
        case String:
            return "";
        case DoubleList:
            return "parseDoubleList";
        case StringList:
            return "parseStringList";
        case IntegerList:
            return "parseIntList";
        default:
            return "";
        }
    }

    static SchemaType CreateFromTypeName(const QString& name)
    {
        SchemaType::Type propType = SchemaType::Custom;
        QString customTypeName;

        if (name == "double")
        {
            propType = SchemaType::Double;
        }
        else if (name == "string")
        {
            propType = SchemaType::String;
        }
        else if (name == "boolean")
        {
            propType = SchemaType::Boolean;
        }
        else if (name == "int")
        {
            propType = SchemaType::Integer;
        }
        else if (name == "dateTime")
        {
            propType = SchemaType::DateTime;
        }
        else if (name == "ListOfDouble")
        {
            propType = SchemaType::DoubleList;
        }
        else if (name == "ListOfString")
        {
            propType = SchemaType::StringList;
        }
        else if (name == "ListOfInt")
        {
            propType = SchemaType::IntegerList;
        }
        else
        {
            customTypeName = name;
            propType = SchemaType::Custom;
        }

        if (propType == SchemaType::Custom)
        {
            return SchemaType(customTypeName);
        }
        else
        {
            return SchemaType(propType);
        }
    }

private:
    Type m_type;
    QString m_customType;
};


class Property
{
public:
    Property(const QString& name, const SchemaType& type, int minOccurs = 1, int maxOccurs = 1) :
       m_type(type),
       m_name(name),
       m_minOccurs(minOccurs),
       m_maxOccurs(maxOccurs)
    {
    }

    Property(const Property& other) :
        m_type(other.m_type),
        m_name(other.m_name),
        m_minOccurs(other.m_minOccurs),
        m_maxOccurs(other.m_maxOccurs)
    {
    }

    Property& operator=(const Property& other)
    {
        m_type = other.m_type;
        m_name = other.m_name;
        m_minOccurs = other.m_minOccurs;
        m_maxOccurs = other.m_maxOccurs;

        return *this;
    }

    SchemaType type() const
    {
        return m_type;
    }

    QString name() const
    {
        return m_name;
    }

    QString memberVariableName() const
    {
        return "m_" + name();
    }

    QString accessorName() const
    {
        return name();
    }

    QString mutatorName() const
    {
        return "set" +  capitalizeFirst(name());
    }

    int minOccurs() const
    {
        return m_minOccurs;
    }

    int maxOccurs() const
    {
        return m_maxOccurs;
    }

    enum {
        UNBOUNDED = -1
    };

    bool multipleOccurrencesAllowed() const
    {
        return m_maxOccurs == UNBOUNDED || m_maxOccurs > 1;
    }

private:
    SchemaType m_type;
    QString m_name;
    int m_minOccurs;
    int m_maxOccurs;
};


class Element
{
public:
    Element(const QString& name, const SchemaType& type) :
        m_name(name),
        m_type(type),
        m_defaultValue(""),
        m_minOccurs(1),
        m_maxOccurs(1)
    {
    }

    Element(const Element& other) :
        m_name(other.m_name),
        m_type(other.m_type),
        m_defaultValue(other.m_defaultValue),
        m_minOccurs(other.m_minOccurs),
        m_maxOccurs(other.m_maxOccurs)
    {
    }

    Element& operator=(const Element& other)
    {
        m_name         = other.m_name;
        m_type         = other.m_type;
        m_defaultValue = other.m_defaultValue;
        m_minOccurs     = other.m_minOccurs;
        m_maxOccurs     = other.m_maxOccurs;

        return *this;
    }

    QString name() const
    {
        return m_name;
    }

    SchemaType type() const
    {
        return m_type;
    }

    QString defaultValue() const
    {
        return m_defaultValue;
    }

    void setDefaultValue(const QString& defaultValue)
    {
        m_defaultValue = defaultValue;
    }

    int minOccurs() const
    {
        return m_minOccurs;
    }

    void setMinOccurs(int minOccurs)
    {
        m_minOccurs = minOccurs;
    }

    int maxOccurs() const
    {
        return m_maxOccurs;
    }

    void setMaxOccurs(int maxOccurs)
    {
        m_maxOccurs = maxOccurs;
    }

    enum {
        UNBOUNDED = -1
    };

private:
    QString m_name;
    SchemaType m_type;
    QString m_defaultValue;
    int m_minOccurs;
    int m_maxOccurs;
};


class ComplexType
{
public:
    ComplexType(const QString& name, const QString& baseTypeName = "") :
        m_name(name),
        m_baseTypeName(baseTypeName)
    {
    }

    const QList<Property>& properties() const
    {
        return m_properties;
    }

    QString name() const
    {
        return m_name;
    }

    QString baseTypeName() const
    {
        return m_baseTypeName;
    }

    QString className() const
    {
        return CLASS_PREFIX + name();
    }

    QString baseClassName() const
    {
        if (baseTypeName().isEmpty())
        {
            return CLASS_PREFIX + "Object";
        }
        else
        {
            return CLASS_PREFIX + baseTypeName();
        }
    }

    bool isEmpty() const
    {
        return m_properties.isEmpty();
    }

    void addProperty(const Property& property)
    {
        m_properties << property;
    }

    void writeClassDefinition(QTextStream& out);
    void writeMethodDefinitions(QTextStream& out);
    void writeForwardDeclaration(QTextStream& out);

private:
    void writeMemberVariables(QTextStream& out);
    void writePublicInterface(QTextStream& out);
    void writeDefaultConstructor(QTextStream& out);
    void writeDomLoader(QTextStream& out);

private:
    QString m_name;
    QString m_baseTypeName;
    QList<Property> m_properties;
};


QHash<QString, QSharedPointer<SchemaType> > g_GlobalTypes;
QHash<QString, QSharedPointer<Element> > g_GlobalElements;
QList<QSharedPointer<ComplexType> > g_AllSchemaTypes;


void
ComplexType::writeForwardDeclaration(QTextStream& out)
{
    out << "class " << className() << ";\n";
}


void
ComplexType::writeClassDefinition(QTextStream& out)
{
    out << "class " << className() << " : public " << baseClassName() << "\n";
    out << "{\n";

    writePublicInterface(out);
    out << "\n";
    writeMemberVariables(out);

    out << "};\n";
}


void
ComplexType::writeMethodDefinitions(QTextStream& out)
{
    writeDefaultConstructor(out);
    out << "\n";
    writeDomLoader(out);
    out << "\n\n";
}


void
ComplexType::writeMemberVariables(QTextStream& out)
{
    out << "private:\n";
    foreach (Property p, m_properties)
    {
        if (p.multipleOccurrencesAllowed())
        {
            out << INDENT << p.type().memberVariableVectorType() << " " << p.memberVariableName() << ";\n";
        }
        else
        {
            out << INDENT << p.type().memberVariableType() << " " << p.memberVariableName() << ";\n";
        }
    }
}


void
ComplexType::writePublicInterface(QTextStream& out)
{
    out << "public:\n";

    // Default constructor
    out << INDENT << className() << "();\n";

    // DOM Node loader
    out << INDENT << "void " << "load(const QDomElement& e);\n";
    out << "\n";

    // Accessors and mutators are all defined inline
    foreach (Property p, m_properties)
    {
        // accessor
        if (p.type().isArrayType())
        {
            // Create separate const and non-const array accessors
            out << INDENT << "const " << p.type().memberVariableType() << "& " << p.accessorName() << "() const\n";
            out << INDENT << "{ " << "return " << p.memberVariableName() << "; }\n";
            out << INDENT << p.type().memberVariableType() << "& " << p.accessorName() << "()\n";
            out << INDENT << "{ " << "return " << p.memberVariableName() << "; }\n";
        }
        else if (p.multipleOccurrencesAllowed())
        {
            // Create separate const and non-const array accessors
            out << INDENT << "const " << p.type().memberVariableVectorType() << "& " << p.accessorName() << "() const\n";
            out << INDENT << "{ " << "return " << p.memberVariableName() << "; }\n";
            out << INDENT << p.type().memberVariableVectorType() << "& " << p.accessorName() << "()\n";
            out << INDENT << "{ " << "return " << p.memberVariableName() << "; }\n";
        }
        else
        {
            out << INDENT << p.type().memberVariableType() << " " << p.accessorName() << "() const\n";
            out << INDENT << "{ " << "return " << p.memberVariableName() << "; }\n";
        }

        // mutator
        if (p.multipleOccurrencesAllowed())
        {
            out << INDENT << "void " << p.mutatorName() << "(" << p.type().memberVariableVectorType() << " " << p.name() << ")\n";
            out << INDENT << "{ " << p.memberVariableName() << " = " << p.name() << "; }\n";
        }
        else
        {
            out << INDENT << "void " << p.mutatorName() << "(" << p.type().memberVariableType() << " " << p.name() << ")\n";
            out << INDENT << "{ " << p.memberVariableName() << " = " << p.name() << "; }\n";
        }
    }
}



void
ComplexType::writeDefaultConstructor(QTextStream& out)
{
    out << className() << "::" << className() << "()";

    bool initializerList = false;
    foreach (Property p, m_properties)
    {
        QString defaultValue;

        switch (p.type().type())
        {
        case SchemaType::Double:
            defaultValue = "0.0";
            break;
        case SchemaType::Integer:
            defaultValue = "0";
            break;
        default:
            break;
        }

        if (!defaultValue.isEmpty())
        {
            if (!initializerList)
            {
                initializerList = true;
                out << " :\n";
            }
            else
            {
                out << ",\n";
            }

            out << INDENT << p.memberVariableName() << "(" << defaultValue << ")";
        }
    }

    out << "\n";
    out << "{\n";
    out << "}\n";
}


void
ComplexType::writeDomLoader(QTextStream& out)
{
    out << "void " << className() << "::load(const QDomElement& e)\n";
    out << "{\n";

    out << INDENT << baseClassName() << "::load(e);\n";

    foreach (Property p, m_properties)
    {
        QString child = "e.firstChildElement(" + quoteString(p.name()) + ")";
        if (p.type().isCustom())
        {
            if (p.multipleOccurrencesAllowed())
            {
                out << INDENT << "{\n";
                out << INDENT2 << "QDomElement f = " << child << ";";
                out << INDENT2 << "while (!f.isNull())\n";
                out << INDENT2 << "{\n";
                out << INDENT3 << p.type().memberVariableType() << " v = " << p.type().memberVariableType() << "(new " << p.type().ctype() << "());\n";
                out << INDENT3 << "v->load(" << child << ");\n";
                out << INDENT3 << p.memberVariableName() << " << v;\n";
                out << INDENT2 << "}\n";
            }
            else
            {
                out << INDENT << p.memberVariableName() << " = " << p.type().memberVariableType() << "(new " << p.type().ctype() << "());\n";
                out << INDENT << p.memberVariableName() << "->load(" << child << ");\n";
            }
        }
        else
        {
            out << INDENT << p.memberVariableName() << " = " << p.type().parsingFunction() << "(" << child;
            if (!p.type().isCustom())
            {
                out << ".text()";
            }
            out << ");\n";
        }
    }

    out << "}\n";
}


ComplexType* CreateComplexType(QDomElement e)
{
    QDomElement parentElement = e.parentNode().toElement();
    Q_ASSERT(!parentElement.isNull());

    bool parentIsElement = parentElement.tagName() == "element";

    QString typeName = e.attributes().namedItem("name").nodeValue();
    if (typeName.isEmpty())
    {
        // If no name for the complexType is specified, use the name of the
        // parent element.
        if (parentIsElement)
        {
            typeName = parentElement.attributes().namedItem("name").nodeValue();
        }
    }

    // No name means that there's a problem in the schema
    if (typeName.isEmpty())
    {
        std::cerr << "Missing name for complexType.\n";
        return NULL;
    }

    QDomElement first = e.firstChildElement();
    QDomElement sequence;
    QString base;
    if (first.tagName() == "sequence")
    {
        sequence = first;
    }
    else if (first.tagName() == "complexContent")
    {
        first = first.firstChildElement();
        if (first.tagName() == "sequence")
        {
            sequence = first;
        }
        else if (first.tagName() == "extension")
        {
            base = stripNamespace(first.attribute("base"));
            sequence = first.firstChildElement();
            if (sequence.tagName() != "sequence" && !sequence.isNull())
            {
                std::cerr << "Unrecognized complexType definition pattern for " << typeName.toAscii().data() << "\n";
                return NULL;
            }
        }
        else
        {
            std::cerr << "Unrecognized complexType definition pattern for " << typeName.toAscii().data() << "\n";
            return NULL;
        }
    }

    ComplexType* complexType = new ComplexType(typeName, base);


    QDomNodeList children = sequence.childNodes();
    for (uint i = 0; i < children.length(); ++i)
    {
        QDomNode child = children.item(i);
        if (child.isElement())
        {
            QDomElement e = child.toElement();
            if (e.tagName() == "element")
            {
                QString name = e.attribute("name");
                QString ref = e.attribute("ref");

                int minOccurs = 1;
                int maxOccurs = 1;
                if (e.hasAttribute("minOccurs"))
                {
                    minOccurs = e.attribute("minOccurs").toInt();
                }
                if (e.hasAttribute("maxOccurs"))
                {
                    if (e.attribute("maxOccurs").toLower() == "unbounded")
                    {
                        maxOccurs = Property::UNBOUNDED;
                    }
                    else
                    {
                        maxOccurs = e.attribute("maxOccurs").toInt();
                    }
                }

                if (!ref.isEmpty())
                {
                    QString refElementName = stripNamespace(ref);
                    if (!g_GlobalElements.contains(refElementName))
                    {
                        std::cerr << "Undefined element " << refElementName.toAscii().data() << " in ref.\n";
                    }
                    else
                    {
                        QSharedPointer<Element> refElement = g_GlobalElements[refElementName];
                        Property property(refElement->name(), refElement->type(), minOccurs, maxOccurs);
                        complexType->addProperty(property);
                    }
                }
                else if (!name.isEmpty())
                {
                    QString type = stripNamespace(e.attribute("type"));
                    if (type.isEmpty())
                    {
                        // Anonymous type; use element name as type.
                        type = name;
                    }
                    SchemaType schemaType = SchemaType::CreateFromTypeName(type);
                    Property property(name, schemaType, minOccurs, maxOccurs);
                    complexType->addProperty(property);
                }
            }
        }
    }

    return complexType;
}


QSet<QString> g_LoadedSchemaFiles;

bool ParseSchemaFile(const QString& filename)
{
    QFile in(filename);
    if (!in.open(QIODevice::ReadOnly))
    {
        std::cerr << "Error opening schema file " << filename.toAscii().data() << std::endl;
        return false;
    }

    QDomDocument doc("STA Schema");
    if (!doc.setContent(&in))
    {
        std::cerr << "Error parsing schema file " << filename.toAscii().data() << std::endl;
        return false;
    }

    QDomNodeList includes = doc.elementsByTagName("include");
    for (uint i = 0; i < includes.length(); ++i)
    {
        QDomElement e = includes.item(i).toElement();
        QString schemaLocation = e.attribute("schemaLocation");
        if (!schemaLocation.isEmpty())
        {
            QDir currentFileDir = QFileInfo(in).dir();
            QString includeFile = currentFileDir.canonicalPath() + "/" + schemaLocation;
            if (!g_LoadedSchemaFiles.contains(includeFile))
            {
                std::cerr << "Include: " << includeFile.toAscii().data() << std::endl;
                g_LoadedSchemaFiles.insert(includeFile);
                bool ok = ParseSchemaFile(includeFile);
                if (!ok)
                {
                    return false;
                }
            }
        }
    }


    QDomNodeList elements;

    // Update the global elements table
    elements = doc.elementsByTagName("element");
    for (uint i = 0; i < elements.length(); ++i)
    {
        QDomElement e = elements.item(i).toElement();
        if (e.parentNode().toElement().tagName() == "schema")
        {
            QString name = e.attribute("name");
            if (g_GlobalElements.contains(name))
            {
                std::cerr << "Element " << name.toAscii().data() << " is defined more than once.\n";
            }
            else
            {
                QString type = stripNamespace(e.attribute("type"));
                if (type.isEmpty())
                {
                    // The element uses an anonymous type; assign the type the same
                    // name as the element.
                    type = name;
                }

                SchemaType schemaType = SchemaType::CreateFromTypeName(type);
                QSharedPointer<Element> element(new Element(name, schemaType));
                g_GlobalElements.insert(name, element);
                //std::cerr << "ELEMENT: " << name.toAscii().data() << " => " << type.toAscii().data() << std::endl;
            }
        }
    }

    elements = doc.elementsByTagName("complexType");
    for (uint i = 0; i < elements.length(); ++i)
    {
        QDomElement e = elements.item(i).toElement();

        QSharedPointer<ComplexType> complexType(CreateComplexType(e));
        if (!complexType.isNull())
        {
            g_AllSchemaTypes << complexType;
        }

        if (e.parentNode().toElement().tagName() == "schema")
        {
            QString name = e.attribute("name");
            QSharedPointer<SchemaType> schemaType(new SchemaType(complexType->name()));
            g_GlobalTypes.insert(name, schemaType);
        }
    }

    return true;
}


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QStringList args = app.arguments();

    QStringList schemaFiles = args;
    schemaFiles.removeAt(0);

    // Parse all the schema files given on the command line
    foreach (QString filename, schemaFiles)
    {
        ParseSchemaFile(filename);
    }

    QString outputFile = "staschema";
    QString headerFileName = outputFile + ".h";
    QString sourceFileName = outputFile + ".cpp";
    QFile headerFile(headerFileName);
    QFile sourceFile(sourceFileName);

    if (!headerFile.open(QIODevice::WriteOnly))
    {
        std::cerr << "Error creating header file.\n";
        return 1;
    }

    if (!sourceFile.open(QIODevice::WriteOnly))
    {
        std::cerr << "Error creating source file.\n";
        return 1;
    }


    // Create the header file
    QTextStream header(&headerFile);

    // Write the header file preamble
    header << "class QDomElement;\n";

    header << "#include \"ScenarioParse.h\"\n";
    header << "\n";

    header << "class ScenarioObject\n";
    header << "{\n";
    header << INDENT << "public:\n";
    header << INDENT << "void load(const QDomElement& /* e */) {}\n";
    header << "};\n";
    header << "\n";

    header << "// Forward declarations\n\n";
    foreach (QSharedPointer<ComplexType> e, g_AllSchemaTypes)
    {
        e->writeForwardDeclaration(header);
    }
    header << "\n\n";

    header << "// Space scenario class definitions\n\n";
    foreach (QSharedPointer<ComplexType> e, g_AllSchemaTypes)
    {
        header << "// " << e->className() << "\n";
        e->writeClassDefinition(header);
        header << "\n\n";
    }

    headerFile.close();
    // Header complete


    // Create the source file
    QTextStream source(&sourceFile);

    // Write the preamble
    source << "#include <QtXml>\n";
    source << "#include " << quoteString(headerFileName) << "\n";
    source << "\n";

    foreach (QSharedPointer<ComplexType> e, g_AllSchemaTypes)
    {
        source << "// " << e->className() << "\n";
        e->writeMethodDefinitions(source);
        source << "\n\n";
    }

    sourceFile.close();
    // Source file complete

    //return app.exec();

    return 0;
}

