#include "pch.h"
#include "SignalData.h"

using namespace TimeDelayEstimation;

SignalData::SignalData(const std::vector<SignalType>* channel0, const std::vector<SignalType>* channel1,
    bool copyData)
	: m_copy(copyData), m_min(0), m_max(min(channel0->size(),channel1->size())), m_alignment(0)
{
	if (copyData)
	{
		m_channel0 = new std::vector<SignalType>(*channel0);
		m_channel1 = new std::vector<SignalType>(*channel1);
	}
	else
	{
		m_channel0 = channel0;
		m_channel1 = channel1;
	}
}

SignalData::SignalData(const std::vector<SignalType>* channel0, const std::vector<SignalType>* channel1,
    std::size_t minPos, std::size_t maxPos, bool copyData)
	: m_copy(copyData), m_min(minPos), m_max(maxPos), m_alignment(0)
{
	if (copyData)
	{
		m_channel0 = new std::vector<SignalType>(*channel0);
		m_channel1 = new std::vector<SignalType>(*channel1);
	}
	else
	{
		m_channel0 = channel0;
		m_channel1 = channel1;
	}
	if (minPos >= channel0->size() || minPos >= channel1->size()) m_min = min(channel0->size() - 1, channel1->size() - 1);
	if (maxPos >= channel0->size() || maxPos >= channel1->size()) m_max = min(channel0->size() - 1, channel1->size() - 1);
}

SignalData::~SignalData() 
{
	if (m_copy) 
    {
		delete m_channel0;
		delete m_channel1;
	}
}

SignalValue SignalData::Value(size_t position) const
{
	if (position < 0 || position >= m_channel0->size()) return SignalZero;
	return m_channel0->at(position).value;
}

UINT64 SignalData::Delta(UINT64 i0, UINT64 i1) const
{
	return i0 > i1 ? i0 - i1 : i1 - i0;
}

bool SignalData::DataItem0(size_t position, AudioDataItem* item) const
{
	if (position < 0 || position >= m_channel0->size()) return false;
	*item = m_channel0->at(position);
	return true;
}

bool SignalData::DataItem1(size_t position, AudioDataItem* item, UINT64 ts0) const
{
	if (position < 0 || position >= m_channel1->size()) return false;

	if (ts0 == 0)
	{
		*item = m_channel1->at(position);
		return true;
	}

	UINT64 ts1 = m_channel1->at(position).timestamp;

	if (ts0 == ts1)
	{
		*item = m_channel1->at(position);
		return true;
	}

	if (ts0 < ts1)
	{
		size_t p = position;
		while (p > 0 && Delta(ts0, m_channel1->at(p - 1).timestamp) < Delta(ts0, m_channel1->at(p).timestamp)) { p--; }
		if (p < (m_channel1->size() - 1) && Delta(ts0, m_channel1->at(p).timestamp) >= Delta(ts0, m_channel1->at(p + 1).timestamp)) { p++; }
		*item = m_channel1->at(p);
		return true;
	}
	else
	{
		size_t p = position;
		size_t sz = m_channel1->size() - 1;
		while (p < sz && Delta(ts0, m_channel1->at(p + 1).timestamp) < Delta(ts0, m_channel1->at(p).timestamp)) { p++; }
		if (p > 0 && Delta(ts0, m_channel1->at(p).timestamp) >= Delta(ts0, m_channel1->at(p - 1).timestamp)) { p--; }
		*item = m_channel1->at(p);
		return true;
	}
}

bool SignalData::CalculateAlignment(size_t position, DelayType* alignment, UINT64* delta)
{
	DelayType p = (DelayType)position;

	if (position >= m_channel0->size()) return false;
	if (position >= m_channel1->size()) return false;

	UINT64 ts0 = m_channel0->at(position).timestamp;
	UINT64 ts1 = m_channel1->at(position).timestamp;

	if (ts0 > ts1)
	{
		while (m_channel1->at(p).timestamp < ts0) { if (++p >= (DelayType)m_channel1->size()) return false;	}
		if (p > 0 && ts0 - m_channel1->at(p - 1).timestamp > m_channel1->at(p).timestamp - ts0) { p--; }
	}
	else if (ts0 < ts1)
	{
		while (m_channel1->at(p).timestamp > ts0) { if (--p < 0) return false; }
		if (p < (DelayType)(m_channel1->size() - 1) && ts0 - m_channel1->at(p).timestamp > m_channel1->at(p + 1).timestamp - ts0) { p++; }
	}
	if (alignment != NULL) *alignment = p - (DelayType)position;
	if (delta != NULL) *delta = m_channel1->at(p).timestamp - ts0;
	return true;
}
