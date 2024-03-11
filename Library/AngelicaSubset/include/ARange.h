#ifndef __ARANGE_H__
#define __ARANGE_H__

#include <algorithm>
#include <cmath>
#include <compare>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <random>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <vector>

template<typename T>
class ARange
{
public:
	class Iterator
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = T;
		using difference_type = std::ptrdiff_t;
		using pointer = T*;
		using reference = T&;

		explicit Iterator(T value, T step) : value_(value), step_(step) {}

		T operator*() const { return value_; }
		Iterator& operator++() { value_ += step_; return *this; }
		Iterator operator++(int) { Iterator temp = *this; ++(*this); return temp; } // Post-increment
		Iterator& operator--() { value_ -= step_; return *this; }
		Iterator operator--(int) { Iterator temp = *this; --(*this); return temp; } // Post-decrement
		Iterator operator+(size_t n) const { return Iterator(value_ + n * step_, step_); } // Addition
		Iterator operator-(size_t n) const { return Iterator(value_ - n * step_, step_); } // Subtraction
		Iterator& operator+=(size_t n) { value_ += n * step_; return *this; } // Compound addition
		Iterator& operator-=(size_t n) { value_ -= n * step_; return *this; } // Compound subtraction
		T operator[](size_t n) const { return value_ + n * step_; } // random access

		bool operator!=(const Iterator& other) const { return value_ != other.value_; }
		bool operator==(const Iterator& other) const { return value_ == other.value_; }
		bool operator<(const Iterator& other) const { return value_ < other.value_; }
		bool operator>(const Iterator& other) const { return value_ > other.value_; }
		bool operator<=(const Iterator& other) const { return value_ <= other.value_; }
		bool operator>=(const Iterator& other) const { return value_ >= other.value_; }
	
	private:
		T value_;
		T step_;
	};
	Iterator begin() const { return Iterator(begin_, step_); }
	Iterator begin(T stride) const { return Iterator(begin_, stride); }
	Iterator end() const { return Iterator(end_ + step_, step_); }
	
	Iterator rbegin() const { return Iterator(end_ - step_, -step_); }
	Iterator rbegin(T stride) const { return Iterator(end_ - step_, -stride); }
	Iterator rend() const { return Iterator(begin_ - step_, -step_); }

public:
	// Constructors
	ARange() : begin_(0), end_(0), step_(1) {}
	ARange(const std::vector<std::unique_ptr<T>>& v) : data_(v) {}

	// Constructor accepting rvalue reference to initialize elements
	ARange(std::vector<std::unique_ptr<T>>&& v) : data_(std::move(v)) {}

	ARange(const std::vector<T>& v) : data_(v) {}

	// Constructor accepting iterators to initialize elements
	template<typename InputIt>	
	ARange(InputIt begin, InputIt end)
	{
		for (auto it = begin; it != end; ++it)
			data_.push_back(std::make_unique<T>(std::move(*it)));
	}

	ARange(T start, T stop, T step = 1)
		: begin_(start)
		, end_(stop)
		, step_(step)
	{
		if (!isValidRange())
			throw std::invalid_argument("Invalid range: begin must be less than or equal to end.");
		
		if (step_ <= 0)
			throw std::invalid_argument("Step size must be greater than zero.");
	}

	// Getters
	const T& get_begin() const { return begin_; }
	const T& get_end() const { return end_; }
	const T& get_step() const { return step_; }

	T* get_begin_ptr() { return &begin_; }
	T* get_end_ptr() { return &end_; }
	const T* get_begin_ptr() const { return &begin_; }
	const T* get_end_ptr() const { return &end_; }

	// Range properties and operations
	T size() const
	{
		return (T)((end_ - begin_) / step_) + 1;
	}

	// Resize the range while preserving its direction
	void resize(T newBegin, T newEnd)
	{
		if (newBegin <= newEnd)
		{
			begin_ = newBegin;
			end_ = newEnd;
		}
		else
		{
			begin_ = newEnd;
			end_ = newBegin;
		}
	}

	void resizeStart(T newBegin)
	{
		if (newBegin <= end_)
			begin_ = newBegin;
		else
			throw std::invalid_argument("New begin value must be less than or equal to current end value.");
	}

	void resizeStop(T newEnd)
	{
		if (begin_ <= newEnd)
			end_ = newEnd;
		else
			throw std::invalid_argument("New end value must be greater than or equal to current begin value.");
	}

	void resizeStep(T newStep)
	{
		if (newStep > 0)
			step_ = newStep;
		else
			throw std::invalid_argument("Step size must be greater than zero.");
	}

	template<typename U>
	bool contains(const U& value) const
	{
		// Check if the value falls within the range
		if (value >= begin_ && value <= end_)
		{
			// Check if the difference between the value and the start of the range
			// is a multiple of the step size, within a small epsilon range to account for precision issues
			return std::abs((value - begin_) / step_ - std::round((value - begin_) / step_)) < std::numeric_limits<U>::epsilon();
		}
		return false;
	}

	template<typename U>
	bool overlaps(const ARange<U>& other) const
	{
		return !(end_ < other.get_begin() || other.get_end() < begin_);
	}

	template<typename U>
	ARange<typename std::common_type<T, U>::type> intersection(const ARange<U>& other) const
	{
		using CommonType = typename std::common_type<T, U>::type;
		if (!overlaps(other))
			return ARange<CommonType>(0, 0);

		CommonType intersection_begin = std::max(static_cast<CommonType>(begin_), static_cast<CommonType>(other.get_begin()));
		CommonType intersection_end = std::min(static_cast<CommonType>(end_), static_cast<CommonType>(other.get_end()));
		return ARange<CommonType>(intersection_begin, intersection_end);
	}

	template<typename U>
	ARange<typename std::common_type<T, U>::type> union_with(const ARange<U>& other) const
	{
		return ARange<typename std::common_type<T, U>::type>(std::min(begin_, other.get_begin()), std::max(end_, other.get_end()));
	}

	ARange<T> subrange(const T& start, const T& stop) const
	{
		return ARange<T>(std::max(begin_, start), std::min(end_, stop), step_);
	}

	ARange<T> merge(const ARange<T>& other) const
	{
		return ARange<T>(std::min(begin_, other.begin_), std::max(end_, other.end_), std::min(step_, other.step_));
	}

	std::vector<ARange<T>> split() const
	{
		std::vector<ARange<T>> result;
		result.push_back(*this);
		return result;
	}

	T operator[](size_t index) const
	{
		if (index >= size())
			throw std::out_of_range("Index out of range");
		
		return begin_ + index * step_;
	}

	// Arithmetic operations
	ARange<T> operator+(const T& value) const
	{
		return ARange<T>(begin_ + value, end_ + value);
	}

	ARange<T> operator-(const T& value) const
	{
		return ARange<T>(begin_ - value, end_ - value);
	}

	ARange<T> operator*(const T& value) const
	{
		return ARange<T>(begin_ * value, end_ * value);
	}

	ARange<T> operator/(const T& value) const
	{
		if (value == 0)
			return ARange<T>(0, 0); // division by zero, return an invalid range
		
		return ARange<T>(begin_ / value, end_ / value);
	}
	
	// Comparison operators
	bool operator==(const ARange<T>& other) const
	{
		return begin_ == other.begin_ && end_ == other.end_ && step_ == other.step_;
	}

	bool operator!=(const ARange<T>& other) const
	{
		return !(*this == other);
	}

	bool operator<(const ARange<T>& other) const
	{
		return std::tie(begin_, end_, step_) < std::tie(other.begin_, other.end_, other.step_);
	}

	bool operator>(const ARange<T>& other) const
	{
		return std::tie(begin_, end_, step_) > std::tie(other.begin_, other.end_, other.step_);
	}

	bool operator<=(const ARange<T>& other) const
	{
		return std::tie(begin_, end_, step_) <= std::tie(other.begin_, other.end_, other.step_);
	}

	bool operator>=(const ARange<T>& other) const
	{
		return std::tie(begin_, end_, step_) >= std::tie(other.begin_, other.end_, other.step_);
	}

	std::strong_ordering operator<=>(const ARange<T>& other) const
	{
		return std::tie(begin_, end_, step_) <=> std::tie(other.begin_, other.end_, other.step_);
	}

	// Stream insertion and extraction operators
	std::string to_string() const
	{
		// Serialize the range to a string representation
		std::ostringstream oss;
		oss << "[" << begin_ << ", " << end_ << ", " << step_ << "]";
		return oss.str();
	}

	static ARange<T> from_string(const std::string& str)
	{
		// Deserialize a string to an ARange object
		T begin, end, step;
		char delim;
		std::istringstream iss(str);

		// Read the opening bracket
		if (!(iss >> delim) || delim != '[')
			throw std::invalid_argument("Invalid range format: missing opening bracket.");

		// Read the values, allowing for negative values and different delimiters
		if (!(iss >> begin >> delim))
			throw std::invalid_argument("Invalid range format: unable to read begin value.");
		if (!(iss >> end >> delim))
			throw std::invalid_argument("Invalid range format: unable to read end value.");
		if (!(iss >> step >> delim))
			throw std::invalid_argument("Invalid range format: unable to read step value.");

		// Read the closing bracket
		if (!(iss >> delim) || delim != ']')
			throw std::invalid_argument("Invalid range format: missing closing bracket.");

		return ARange<T>(begin, end, step);
	}
	
	// Other utility methods
	T sum() const
	{
		// Calculate the number of terms in the range
		T numTerms = size();

		// Calculate the sum using the arithmetic sum formula
		T firstTerm = begin_;
		T lastTerm = begin_ + (numTerms - 1) * step_;
		T result = (numTerms * (firstTerm + lastTerm)) / 2;

		return (T)result;
	}

	T mean() const
	{
		return sum() / static_cast<T>(size());
	}

	void clear()
	{
		begin_ = 0;
		end_ = 0;
		step_ = 1;
	}

	void expand(T amount)
	{
		end_ += amount;
	}

	void shrink(T amount)
	{
		end_ -= amount;
	}

	T middle() const
	{
		// Calculate the midpoint by adding half of the step size to the lower bound.
		// result is always the floor value of the midpoint, even when the sum of the bounds is odd.
		return begin_ + (end_ - begin_) / 2;
	}

	template<typename U>
	bool containsAll(const ARange<U>& other) const
	{
		return other.begin_ >= begin_ && other.end_ <= end_;
	}

	template<typename U>
	bool containsAny(const ARange<U>& other) const
	{
		return overlaps(other);
	}

	bool isUniform() const
	{
		return step_ != 0 && (begin_ + step_ == end_ || begin_ - step_ == end_);
	}

	void scale(T factor)
	{
		begin_ *= factor;
		end_ *= factor;
		step_ *= factor;
	}

	std::vector<T> to_vector() const
	{
		std::vector<T> result;
		for (T value = begin_; (step_ > 0 && value <= end_) || (step_ < 0 && value >= end_); value += step_)
		{
			// Check if the absolute difference between 'value' and 'end_' is within a small epsilon range
			if (std::abs(value - end_) < std::numeric_limits<T>::epsilon())
				break; // Exit the loop if 'value' is very close to 'end_'

			result.push_back(value);
		}
		return result;
	}

	enum STEP_SIZE
	{
		EQUAL,
		MINIMUM,
		DYNAMIC
	};

	static ARange<T> from_vector(const std::vector<T>& vec, STEP_SIZE stepSize = DYNAMIC)
	{
		if (vec.empty())
			throw std::invalid_argument("Cannot create range from empty vector");
		
		// Initialize minVal and maxVal with the first element of the vector
		T minVal = vec[0];
		T maxVal = vec[0];

		// Iterate through the vector to find the minimum and maximum values
		for (const T& val : vec)
		{
			if (val < minVal)
				minVal = val; // Update minVal if val is smaller
			else if (val > maxVal)
				maxVal = val; // Update maxVal if val is larger
		}

		// Calculate the step size based on the difference between minVal and maxVal
		T step;
		switch (stepSize)
		{
		case EQUAL:
			step = (maxVal - minVal) / (vec.size() - 1);
			break;
		case MINIMUM:
			step = std::max((maxVal - minVal) / (vec.size() - 1), static_cast<T>(1));
			break;
		case DYNAMIC:
		default:
			step = 1; // Default to step 1, use 'resizeStep' to change, if necessary
			break;
		}
		
		return ARange<T>(minVal, maxVal, step);
	}

	// Iterators
	// This iterator will iterate over the range in reverse order,
	// starting from the end and moving towards the beginning.
	template <typename T>
	class ReverseIterator
	{
	public:
		ReverseIterator(T value, T step) : value_(value), step_(step) {}

		T operator*() const { return value_; }
		ReverseIterator& operator++() { value_ -= step_; return *this; }
		bool operator!=(const ReverseIterator& other) const { return value_ != other.value_; }

	private:
		T value_;
		T step_;
	};

	// This iterator will iterate over the range with a specified stride,
	// skipping elements based on the given stride value.
	template <typename T>
	class StridedIterator
	{
	public:
		StridedIterator(T value, T step) : value_(value), step_(step) {}

		T operator*() const { return value_; }
		StridedIterator& operator++() { value_ += step_; return *this; }
		bool operator!=(const StridedIterator& other) const { return value_ != other.value_; }

	private:
		T value_;
		T step_;
	};

	// This iterator will iterate over the range with elements filtered based on a predicate.
	template<typename T, typename Predicate>
	class FilteredIterator
	{
	public:
		FilteredIterator(T value, T step, Predicate predicate) : value_(value), step_(step), predicate_(predicate) {}

		T operator*() const { return value_; }

		FilteredIterator& operator++()
		{
			do
			{
				value_ += step_;
			} while (!predicate_(value_));
			return *this;
		}

		bool operator!=(const FilteredIterator& other) const { return value_ != other.value_; }

	private:
		T value_;
		T step_;
		Predicate predicate_;
	};

	// This iterator will iterate over the range with elements transformed by a function.
	template<typename T, typename Function>
	class TransformedIterator
	{
	public:
		TransformedIterator(T value, T step, Function function) : value_(value), step_(step), function_(function) {}

		T operator*() const { return function_(value_); }
		TransformedIterator& operator++() { value_ += step_; return *this; }
		bool operator!=(const TransformedIterator& other) const { return value_ != other.value_; }

	private:
		T value_;
		T step_;
		Function function_;
	};

	// Specific Tange Operations

	// Concatenation: Concatenate two ranges into a single range.
	ARange<T> concat(const ARange<T>& other) const
	{
		return ARange<T>(begin_, other.end_, step_);
	}

	// Partitioning: Partition the range into subranges based on a predicate.
	template<typename Predicate>
	std::vector<ARange<T>> partition(Predicate predicate) const
	{
		std::vector<ARange<T>> partitions;
		T start = begin_;
		T end;
		for (T value = begin_; value <= end_; value += step_)
		{
			if (predicate(value))
			{
				end = value;
				partitions.push_back(ARange<T>(start, end, step_));
				start = value + step_;
			}
		}

		// Add the last partition
		partitions.push_back(ARange<T>(start, end_, step_));
		return partitions;
	}

	// Filtering: Filter elements within the range based on a predicate.
	template<typename Predicate>
	ARange<T> filter(Predicate predicate) const
	{
		T start = begin_;
		T end = end_;
		while (!predicate(start))
		{
			start += step_;
			if (start > end_)
				return ARange<T>(); // Empty range
		}

		while (!predicate(end))
			end -= step_;
		
		return ARange<T>(start, end, step_);
	}

	// Reduction: Perform reduction operations such as sum, product, min, max, etc., over the range.
	template<typename BinaryOperation>
	T reduce(BinaryOperation op, T initial) const
	{
		T result = initial;
		for (T value = begin_; value <= end_; value += step_)
			result = op(result, value);

		return result;
	}

	// Mapping/Transformation: Apply a function to each element of the range to create a new range.
	template<typename Function>
	ARange<std::invoke_result_t<Function, T>> map(Function function) const
	{
		using ResultType = std::invoke_result_t<Function, T>;
		std::vector<ResultType> result;
		for (T value = begin_; value <= end_; value += step_)
			result.push_back(function(value));
		
		return ARange<ResultType>(result.front(), result.back(), step_);
	}
	
	// Generate random indices within the range.
	// Retrieve values at those indices.
	template<typename T>
	std::vector<T> random_sample(const ARange<T>& range, size_t count)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<size_t> dist(0, size() - 1);

		std::vector<T> samples;
		samples.reserve(count);
		for (size_t i = 0; i < count; ++i)
		{
			size_t index = dist(gen);
			samples.push_back((*this)[index]);
		}

		return samples;
	}

	// Iterate over the range, accumulating the sum of values.
	// Store cumulative sums in a separate container.
	template<typename T>
	std::vector<T> cumulative_sum(const ARange<T>& range) const
	{
		std::vector<T> sums;
		sums.reserve(size());

		T sum = 0;
		for (T value : *this)
		{
			sum += value;
			sums.push_back(sum);
		}

		return sums;
	}

	std::vector<T> cumulative_sum2(const ARange<T>& range) const
	{
		Iterator begin = Iterator(begin_, step_);
		Iterator end = Iterator(end_ + step_, step_);
		std::vector<T> sums;
		sums.reserve(size());

		T sum = 0;
		for (Iterator it = begin; it != end; ++it)
		{
			sum += *it;
			sums.push_back(sum);
		}

		return sums;
	}

	// Define bins and count occurrences of values falling into each bin.
	template<typename T>
	std::map<T, size_t> histogram(const ARange<T>& range, size_t num_bins) const
	{
		std::map<T, size_t> hist;
		Iterator begin = range.begin();
		Iterator end = range.end();
		T bin_width = (*end - *begin) / static_cast<T>(num_bins);

		for (Iterator it = begin; it != end; ++it)
		{
			T bin = (*it - *begin) / bin_width;
			hist[bin * bin_width + *begin]++;
		}

		return hist;
	}

	// Scale values within the range to fit within a specified range or distribution.
	template<typename T>
	ARange<T> normalize(const ARange<T>& target_range) const
	{
		Iterator begin = this->begin();
		Iterator end = this->end();

		T min_value = *std::min_element(begin, end);
		T max_value = *std::max_element(begin, end);
		T target_min = *target_range.begin();
		T target_max = *target_range.end();

		ARange<T> normalized;
		normalized.resizeStop(target_min + (target_max - target_min) * (*end - max_value) / (max_value - min_value));
		normalized.resizeStart(target_min - min_value + *normalized.begin());
		normalized.resizeStep((*normalized.end() - *normalized.begin()) / size());
		return normalized;
	}
	
	// Count the occurrences of each unique value in the range.
	// Find the value(s) with the highest count.
	template<typename T>
	T mode(const ARange<T>& range)
	{
		// Count occurrences of each unique value
		std::unordered_map<T, int> counts;
		Iterator begin = range.begin();
		Iterator end = range.end();
		for (auto it = begin; it != end; ++it)
			counts[*it]++;

		// Find the mode(s) with the highest count
		int maxCount = 0;
		std::vector<T> modes;
		for (const auto& pair : counts)
		{
			if (pair.second > maxCount)
			{
				maxCount = pair.second;
				modes.clear();
				modes.push_back(pair.first);
			}
			else if (pair.second == maxCount)
				modes.push_back(pair.first);
		}

		if (modes.empty())
			throw std::runtime_error("No mode found in the range");

		return modes[0]; // Return the first mode found
	}

	template<typename T>
	T mode()
	{
		// Call the mode function with the current range
		return mode(*this);
	}

	template<typename T>
	ARange<T> make_range(T start, T stop, T step = 1)
	{
		return ARange<T>(start, stop, step);
	}

	// Method to begin iteration with transformed values
	template<typename Function>
	TransformedIterator<T, Function> transformedBegin(Function function) const
	{
		return TransformedIterator<T, Function>(begin_, step_, function);
	}

	// Method to end iteration with transformed values
	template<typename Function>
	TransformedIterator<T, Function> transformedEnd() const
	{
		// The end iterator should point one step before the actual end
		return TransformedIterator<T, Function>(end_ + step_, step_, {});
	}

	void push_back(const T& value)
	{
		data_.push_back(value);
	}

	void pop_back(const T& value)
	{
		data_.pop_back(value);
	}

private:
	bool isValidRange() const
	{
		return begin_ <= end_;
	}

	T begin_;
	T end_;
	T step_;
	std::vector<std::unique_ptr<T>> data_;
};

//template<typename T>
//std::ostream& operator<<(std::ostream& os, const ARange<T>& range)
//{
//	os << "[";
//	for (auto it = range.get_begin(); it != range.get_end(); ++it)
//	{
//		os << *it;
//		if (std::next(it) != range.get_end())
//			os << ", ";
//	}
//	os << "]";
//	return os;
//}

template<typename T>
std::ostream& operator<<(std::ostream& os, const ARange<T>& range)
{
	os << "[" << range.get_begin() << ", " << range.get_end() << ", " << range.get_step() << "]";
	return os;
}

template<typename T>
std::istream& operator>>(std::istream& is, ARange<T>& range)
{
	T start, stop, step;
	char c;
	is >> c >> start >> c >> stop >> c >> step >> c;
	range = ARange<T>(start, stop, step);
	return is;
}

#endif
