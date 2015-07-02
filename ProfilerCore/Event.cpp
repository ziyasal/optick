#include <cstring>
#include <windows.h>
#include "Event.h"
#include "Core.h"
#include "Thread.h"
#include "EventDescriptionBoard.h"

namespace Profiler
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static CriticalSection lock;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EventDescription* EventDescription::Create(const char* eventName, const char* fileName, const unsigned long fileLine, const unsigned long eventColor /*= Color::Null*/)
{
	CRITICAL_SECTION(lock)
	EventDescription* result = EventDescriptionBoard::Get().CreateDescription();
	result->name = eventName;
	result->file = fileName;
	result->line = fileLine;
	result->color = eventColor;
	return result;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EventDescription::EventDescription() : name(""), file(""), line(0), color(0), isSampling(false)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EventDescription& EventDescription::operator=(const EventDescription&)
{
	BRO_FAILED("It is pointless to copy EventDescription. Please, check you logic!"); return *this; 
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EventData* Event::Start(const EventDescription& description)
{
	EventData* result = nullptr;

	if (EventStorage* storage = Core::storage)
	{
		result = &storage->NextEvent();
		result->description = &description;
		result->Start();

		if (description.isSampling)
		{
			InterlockedIncrement(&storage->isSampling);
		}
	}
	return result;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Event::Stop(EventData& data)
{
	data.Stop();

	if (data.description->isSampling)
	{
		if (EventStorage* storage = Core::storage)
			InterlockedDecrement(&storage->isSampling);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
OutputDataStream & operator<<(OutputDataStream &stream, const EventDescription &ob)
{
	byte flags = (ob.isSampling ? 0x1 : 0);
	return stream << ob.name << ob.file << ob.line << ob.color << flags;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
OutputDataStream& operator<<(OutputDataStream& stream, const EventTime& ob)
{
	return stream << ob.start << ob.finish;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
OutputDataStream& operator<<(OutputDataStream& stream, const EventData& ob)
{
	return stream << (EventTime)(ob) << ob.description->index;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Category::Category(const EventDescription& description) : Event(description)
{
	if (data)
		if (EventStorage* storage = Core::storage)
			storage->RegisterCategory(*data);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}