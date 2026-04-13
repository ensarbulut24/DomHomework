#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

int main() {
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr root = xmlNewNode(NULL, BAD_CAST "root");
    xmlDocSetRootElement(doc, root);
    xmlNewChild(root, NULL, BAD_CAST "test", BAD_CAST "Ömer");
    
    xmlSaveFormatFileEnc("test_le.xml", doc, "UTF-16LE", 1);
    xmlSaveFormatFileEnc("test_be.xml", doc, "UTF-16BE", 1);
    xmlFreeDoc(doc);
    return 0;
}
