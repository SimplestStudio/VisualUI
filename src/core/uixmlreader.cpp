#include "uixmlreader.h"
#ifdef _WIN32
# include <msxml2.h>
# include <comdef.h>
#else
# include <libxml2/libxml/parser.h>
# include <libxml2/libxml/tree.h>
#endif


/* Xml Node */

class XmlNode::XmlNodePrivate
{
public:
    XmlNodePrivate()
    {}

    XmlNodePrivate(const XmlNodePrivate& other)
    {
        pElem = other.pElem;
#ifdef _WIN32
        if (pElem) {
            pElem->AddRef();
        }
#endif
    }

    XmlNodePrivate(XmlNodePrivate&& other) noexcept
    {
        pElem = other.pElem;
        other.pElem = nullptr;
    }

    ~XmlNodePrivate()
    {
        if (pElem) {
#ifdef _WIN32
            pElem->Release();
#endif
            pElem = nullptr;
        }
    }

    XmlNodePrivate& operator=(const XmlNodePrivate& other)
    {
        if (this != &other) {
#ifdef _WIN32
            if (pElem) {
                pElem->Release();
            }
#endif
            pElem = other.pElem;
#ifdef _WIN32
            if (pElem) {
                pElem->AddRef();
            }
#endif
        }
        return *this;
    }

    XmlNodePrivate& operator=(XmlNodePrivate&& other) noexcept {
        if (this != &other) {
#ifdef _WIN32
            if (pElem) {
                pElem->Release();
            }
#endif
            pElem = other.pElem;
            other.pElem = nullptr;
        }
        return *this;
    }

#ifdef _WIN32
    IXMLDOMElement *pElem = nullptr;
#else
    xmlNode* pElem = nullptr;
#endif
};


XmlNode::XmlNode() :
    pimpl(new XmlNodePrivate)
{

}

XmlNode::XmlNode(const XmlNode& other) :
    pimpl(new XmlNodePrivate(*other.pimpl))
{

}

XmlNode::XmlNode(XmlNode&& other) noexcept :
    pimpl(other.pimpl)
{
    other.pimpl = nullptr;
}

XmlNode::~XmlNode()
{
    if (pimpl)
        delete pimpl, pimpl = nullptr;
}

XmlNode& XmlNode::operator=(const XmlNode& other) {
    if (this != &other) {
        *pimpl = *other.pimpl;
    }
    return *this;
}

XmlNode& XmlNode::operator=(XmlNode&& other) noexcept {
    if (this != &other) {
        delete pimpl;
        pimpl = other.pimpl;
        other.pimpl = nullptr;
    }
    return *this;
}

bool XmlNode::isValid() const noexcept
{
    return pimpl && pimpl->pElem;
}

tstring XmlNode::getTagName() const
{
    if (isValid()) {
#ifdef _WIN32
        BSTR tagName;
        if (SUCCEEDED(pimpl->pElem->get_tagName(&tagName))) {
            tstring name(tagName);
            SysFreeString(tagName);
            return name;
        }
#else
        if (pimpl->pElem->name) {
            return (const char*)pimpl->pElem->name;
        }
#endif
    }
    return {};
}

tstring XmlNode::getText() const
{
    if (isValid()) {
#ifdef _WIN32
        BSTR bstrText = nullptr;
        HRESULT hr = pimpl->pElem->get_text(&bstrText);
        if (SUCCEEDED(hr) && bstrText) {
            tstring text(bstrText);
            SysFreeString(bstrText);
            return text;
        }
#else
        if (xmlChar* content = xmlNodeGetContent(pimpl->pElem)) {
            tstring text((const char*)content);
            xmlFree(content);
            return text;
        }
#endif
    }
    return {};
}

tstring XmlNode::getAttributeValue(const tstring &name) const
{
    if (isValid()) {
#ifdef _WIN32
        VARIANT value;
        VariantInit(&value);
        _bstr_t attrName(name.c_str());
        if (SUCCEEDED(pimpl->pElem->getAttribute(attrName, &value))) {
            if (value.vt == VT_BSTR) {
                tstring valStr(value.bstrVal);
                VariantClear(&value);
                return valStr;
            }
        }
        VariantClear(&value);
#else
        if (pimpl->pElem->type == XML_ELEMENT_NODE) {
            if (xmlChar* value = xmlGetProp(pimpl->pElem, (const xmlChar*)name.c_str())) {
                std::string valStr((const char*)value);
                xmlFree(value);
                return valStr;
            }
        }
#endif
    }
    return {};
}

std::vector<std::pair<tstring, tstring>> XmlNode::getAttributes() const
{
    std::vector<std::pair<tstring, tstring>> vec_attrs;
    if (isValid()) {
#ifdef _WIN32
        IXMLDOMNamedNodeMap* pAttrs = nullptr;
        HRESULT hr = pimpl->pElem->get_attributes(&pAttrs);
        if (SUCCEEDED(hr) && pAttrs) {
            long length = 0;
            hr = pAttrs->get_length(&length);
            if (FAILED(hr))
                length = 0;

            for (long i = 0; i < length; i++) {
                IXMLDOMNode* pAttrNode = nullptr;
                hr = pAttrs->get_item(i, &pAttrNode);
                if (SUCCEEDED(hr) && pAttrNode) {
                    BSTR attrName = nullptr;
                    BSTR attrValue = nullptr;
                    hr = pAttrNode->get_nodeName(&attrName);
                    if (SUCCEEDED(hr) && attrName) {
                        hr = pAttrNode->get_text(&attrValue);
                        if (SUCCEEDED(hr) && attrValue) {
                            vec_attrs.emplace_back(attrName, attrValue);
                            SysFreeString(attrValue);
                        }
                        SysFreeString(attrName);
                    }
                    pAttrNode->Release();
                }
            }
            pAttrs->Release();
        }
#else
        for (xmlAttr* attr = pimpl->pElem->properties; attr != nullptr; attr = attr->next) {
            if (!attr->name) continue;
            const char* name = (const char*)attr->name;
            if (xmlChar* value = xmlNodeListGetString(pimpl->pElem->doc, attr->children, 1)) {
                vec_attrs.emplace_back(name, (const char*)value);
                xmlFree(value);
            }
        }
#endif
    }
    return vec_attrs;
}

std::vector<XmlNode> XmlNode::getChildren() const
{
    std::vector<XmlNode> nodes;
    if (isValid()) {
#ifdef _WIN32
        IXMLDOMNodeList* pChildNodes = nullptr;
        HRESULT hr = pimpl->pElem->get_childNodes(&pChildNodes);
        if (SUCCEEDED(hr) && pChildNodes) {
            long length = 0;
            hr = pChildNodes->get_length(&length);
            if (FAILED(hr))
                length = 0;
            for (long i = 0; i < length; i++) {
                IXMLDOMNode* pChildNode = nullptr;
                hr = pChildNodes->get_item(i, &pChildNode);
                if (SUCCEEDED(hr) && pChildNode) {
                    DOMNodeType nodeType;
                    hr = pChildNode->get_nodeType(&nodeType);
                    if (SUCCEEDED(hr) && nodeType == NODE_ELEMENT) {
                        IXMLDOMElement* pChildElem = nullptr;
                        hr = pChildNode->QueryInterface(IID_PPV_ARGS(&pChildElem));
                        if (SUCCEEDED(hr) && pChildElem) {
                            XmlNode node;
                            node.pimpl->pElem = pChildElem;
                            nodes.push_back(node);
                        }
                    }
                    pChildNode->Release();
                }
            }
            pChildNodes->Release();
        }
#else
        if (pimpl && pimpl->pElem) {
            for (xmlNode* child = pimpl->pElem->children; child; child = child->next) {
                if (child->type == XML_ELEMENT_NODE) {
                    XmlNode node;
                    node.pimpl->pElem = child;
                    nodes.push_back(node);
                }
            }
        }
#endif
    }
    return nodes;
}


/* Xml Reader */

class XmlReader::XmlReaderPrivate
{
public:
    XmlReaderPrivate()
    {
#ifdef _WIN32
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
#else
        xmlInitParser();
#endif
    }

    ~XmlReaderPrivate()
    {
        if (pDoc) {
#ifdef _WIN32
            pDoc->Release();
#else
            xmlFreeDoc(pDoc);
#endif
            pDoc = nullptr;
        }
#ifdef _WIN32
        CoUninitialize();
#else
        xmlCleanupParser();
#endif
    }

    bool initDocument()
    {
        if (pDoc) {
#ifdef _WIN32
            pDoc->Release();
#else
            xmlFreeDoc(pDoc);
#endif
            pDoc = nullptr;
        }
#ifdef _WIN32
        HRESULT hr = CoCreateInstance(__uuidof(DOMDocument30), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDoc));
        if (FAILED(hr) || !pDoc) {
            return false;
        }
        pDoc->put_async(VARIANT_FALSE);
        pDoc->put_validateOnParse(VARIANT_FALSE);
        pDoc->put_resolveExternals(VARIANT_FALSE);
#endif
        return true;
    }

#ifdef _WIN32
    IXMLDOMDocument *pDoc = nullptr;
#else
    xmlDoc* pDoc = nullptr;
#endif
};


XmlReader::XmlReader() :
    pimpl(new XmlReaderPrivate)
{

}

XmlReader::~XmlReader()
{
    delete pimpl, pimpl = nullptr;
}

bool XmlReader::loadFromFile(const tstring &fileName)
{
    if (pimpl->initDocument()) {
#ifdef _WIN32
        VARIANT_BOOL loadSuccess = VARIANT_FALSE;
        HRESULT hr = pimpl->pDoc->load(_variant_t(fileName.c_str()), &loadSuccess);
        if (FAILED(hr) || loadSuccess == VARIANT_FALSE) {
            pimpl->pDoc->Release();
            pimpl->pDoc = nullptr;
            return false;
        }
#else
        pimpl->pDoc = xmlReadFile(fileName.c_str(), nullptr, 0);
        if (!pimpl->pDoc)
            return false;
#endif
    }
    return true;
}

bool XmlReader::loadFromXml(const tstring &xml)
{
    if (pimpl->initDocument()) {
#ifdef _WIN32
        VARIANT_BOOL loadSuccess = VARIANT_FALSE;
        HRESULT hr = pimpl->pDoc->loadXML(_bstr_t(xml.c_str()), &loadSuccess);
        if (FAILED(hr) || loadSuccess == VARIANT_FALSE) {
            pimpl->pDoc->Release();
            pimpl->pDoc = nullptr;
            return false;
        }
#else
        pimpl->pDoc = xmlReadMemory(xml.c_str(), xml.length(), nullptr, "UTF-8", 0);
        if (!pimpl->pDoc)
            return false;
#endif
    }
    return true;
}

XmlNode XmlReader::root() const
{
    XmlNode node;
    if (pimpl && pimpl->pDoc) {
#ifdef _WIN32
        HRESULT hr = pimpl->pDoc->get_documentElement(&node.pimpl->pElem);
        if (FAILED(hr)) {
            node.pimpl->pElem = nullptr;
        }
#else
        node.pimpl->pElem = xmlDocGetRootElement(pimpl->pDoc);
#endif
    }
    return node;
}
