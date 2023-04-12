#pragma once

#include "ActionElement.h"
#include "DecoderElement.h"
#include "ReportElement.h"
#include "TrackerElement.h"
namespace sophon_stream {
namespace element {

class MandatoryLink{
public:
    MandatoryLink(){
        DecoderElement::doSth();
        ActionElement::doSth();
        ReportElement::doSth();
        TrackerElement::doSth();
    }
};

MandatoryLink gMandatoryLink;

} // namespace element
} // namespace sophon_stream
