#ifndef UIXMLREADER_H
#define UIXMLREADER_H

#include "uidefines.h"
#include <vector>


class XmlReader;
class DECL_VISUALUI XmlNode
{
public:
    XmlNode();
    XmlNode(const XmlNode& other);
    XmlNode(XmlNode&& other) noexcept;
    ~XmlNode();

    XmlNode& operator=(const XmlNode& other);
    XmlNode& operator=(XmlNode&& other) noexcept;

    bool isValid() const noexcept;
    tstring getTagName() const;
    tstring getText() const;
    tstring getAttributeValue(const tstring &name) const;
    std::vector<std::pair<tstring, tstring>> getAttributes() const;
    std::vector<XmlNode> getChildren() const;

    bool setText(const tstring &text);
    bool setAttribute(const tstring &name, const tstring &value);
    XmlNode appendChild(const tstring &tagName);

private:
    friend XmlReader;
    class XmlNodePrivate;
    XmlNodePrivate *pimpl;
};


class DECL_VISUALUI XmlReader
{
public:

    XmlReader();
    ~XmlReader();

    bool loadFromFile(const tstring &fileName);
    bool loadFromXml(const tstring &xml);
    XmlNode createDocument(const tstring &rootName);
    XmlNode createElement(XmlNode &parent, const tstring &tagName);
    XmlNode root() const;
    bool saveToFile(const tstring &fileName) const;
    tstring toString() const;

private:
    class XmlReaderPrivate;
    XmlReaderPrivate *pimpl;
};

#endif // UIXMLREADER_H
