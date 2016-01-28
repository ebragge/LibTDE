#pragma once
#include <vector>
#include "AudioDataItem.h"

namespace TimeDelayEstimation {

	typedef long DelayType;
	typedef AudioDataItem SignalType;
	typedef int SignalValue;
	typedef LONG64 CalcType;

	const SignalValue SignalZero = 0;
	const CalcType CalcZero = 0;

	class SignalData
	{
	public:
		SignalData(const std::vector<SignalType>* channel0, const std::vector<SignalType>* channel1, bool copyData = true);
		SignalData(const std::vector<SignalType>* channel0, const std::vector<SignalType>* channel1, size_t minPos, size_t maxPos, bool copyData = true);

		~SignalData();

		size_t First() const { return m_min; }
		size_t Last() const { return m_max - 1; }
		size_t Length() const { return m_max-m_min; }

		bool CalculateAlignment(size_t position, DelayType* alignment, UINT64* delta);
		long Alignment() const { return m_alignment; }
		void SetAlignment(DelayType alignment) { m_alignment = alignment; }

		bool DataItem0(size_t position, AudioDataItem* item) const;
		bool DataItem1(size_t position, AudioDataItem* item, UINT64 ts0) const;

		SignalValue Value(size_t position) const;

	private:
		UINT64 Delta(UINT64 i0, UINT64 i1) const;

		bool m_copy;

		std::size_t m_min;
		std::size_t m_max;
		DelayType m_alignment;

		const std::vector<SignalType>* m_channel0;
		const std::vector<SignalType>* m_channel1;
	};
}
