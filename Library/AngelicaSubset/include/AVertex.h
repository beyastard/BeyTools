#ifndef __AVERTEX_H__
#define __AVERTEX_H__

#include <compare>

// Define a class for 3D vertices
template<typename T>
class Vertex
{
public:
    Vertex() : m_x(0), m_y(0), m_z(0) { m_totalVertices++; }
    Vertex(T x, T y, T z) : m_x(x), m_y(y), m_z(z) { m_totalVertices++; }
    Vertex(const Vertex& other) : m_x(other.m_x), m_y(other.m_y), m_z(other.m_z) { m_totalVertices++; }
    Vertex(Vertex&& other) noexcept : m_x(other.m_x), m_y(other.m_y), m_z(other.m_z)
    {
        // Perform any necessary cleanup or transfer of resources
        // Set the source object to a valid but unspecified state
        other.m_x = other.m_y = other.m_z = 0.0f; // Example: Set to a default state
    }

    static int getTotalVertices() { return m_totalVertices; }
    T getX() const { return m_x; }
    T getY() const { return m_y; }
    T getZ() const { return m_z; }

    Vertex<float> operator+(const Vertex<float>& other) const
    {
        return Vertex<float>(m_x + other.m_x, m_y + other.m_y, m_z + other.m_z);
    }

    Vertex<float> operator-(const Vertex<float>& other) const
    {
        return Vertex<float>(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z);
    }

    Vertex<float> operator*(const Vertex<float>& other) const
    {
        return Vertex<float>(m_x * other.m_x, m_y * other.m_y, m_z * other.m_z);
    }

    Vertex<float> operator/(const Vertex<float>& other) const
    {
        // Check for division by zero
        if (other.m_x == 0 || other.m_y == 0 || other.m_z == 0) {
            // Handle division by zero appropriately
            // For example, throw an exception or return a default value
            // Here, we throw an exception
            throw std::runtime_error("Division by zero");
        }

        return Vertex<float>(m_x / other.m_x, m_y / other.m_y, m_z / other.m_z);
    }

    Vertex<float>& operator+=(const Vertex<float>& other)
    {
        m_x += other.m_x;
        m_y += other.m_y;
        m_z += other.m_z;
        return *this;
    }

    Vertex<float>& operator-=(const Vertex<float>& other)
    {
        m_x -= other.m_x;
        m_y -= other.m_y;
        m_z -= other.m_z;
        return *this;
    }

    // Prefix increment operator (++vertex)
    Vertex& operator++() { ++m_x; ++m_y; ++m_z; return *this; }

    // Prefix decrement operator (--vertex)
    Vertex& operator--() { --m_x; --m_y; --m_z; return *this; }

    // Postfix increment operator (vertex++)
    Vertex operator++(int) { Vertex old = *this; ++m_x; ++m_y; ++m_z; return old; }

    // Postfix decrement operator (vertex--)
    Vertex operator--(int) { Vertex old = *this; --m_x; --m_y; --m_z; return old; }

    // Equality operator (vertex1 == vertex2)
    bool operator==(const Vertex<float>& other) const
    {
        const float epsilon = 1e-6f; // Adjust the tolerance as needed

        return
            (std::abs(m_x - other.m_x) < epsilon) &&
            (std::abs(m_y - other.m_y) < epsilon) &&
            (std::abs(m_z - other.m_z) < epsilon);
    }

    // Not equal operator (vertex1 != vertex2
    bool operator!=(const Vertex<float>& other) const
    {
        return !(*this == other);
    }

    bool operator==(const Vertex<double>& other) const
    {
        const double epsilon = 1e-6; // Adjust the tolerance as needed

        return
            (std::abs(m_x - other.m_x) < epsilon) &&
            (std::abs(m_y - other.m_y) < epsilon) &&
            (std::abs(m_z - other.m_z) < epsilon);
    }

    // Not equal operator (vertex1 != vertex2
    bool operator!=(const Vertex<double>& other) const
    {
        return !(*this == other);
    }

    // Less than operator (vertex1 < vertex2)
    bool operator<(const Vertex& other) const
    {
        return m_x < other.m_x && m_y < other.m_y && m_z < other.m_z;
    }

    // Less than or equal to operator (vertex1 <= vertex2)
    bool operator<=(const Vertex& other) const
    {
        return m_x <= other.m_x && m_y <= other.m_y && m_z <= other.m_z;
    }

    // Greater than operator (vertex1 > vertex2)
    bool operator>(const Vertex& other) const
    {
        return m_x > other.m_x && m_y > other.m_y && m_z > other.m_z;
    }

    // Greater than or equal to operator (vertex1 >= vertex2)
    bool operator>=(const Vertex& other) const
    {
        return m_x >= other.m_x && m_y >= other.m_y && m_z >= other.m_z;
    }

    // Three-way comparison operator (vertex1 <=> vertex2)
    std::strong_ordering operator<=>(const Vertex& other) const
    {
        if (m_x != other.m_x) return m_x <=> other.m_x;
        if (m_y != other.m_y) return m_y <=> other.m_y;
        return m_z <=> other.m_z;
    }

private:
    static int m_totalVertices;
    T m_x, m_y, m_z;
};

// Initialize the static member variable
template<typename T>
int Vertex<T>::m_totalVertices = 0;

// Define equality comparison operators for Vertex class
template<typename T>
bool operator==(const Vertex<T>& lhs, const Vertex<T>& rhs)
{
    return lhs.getX() == rhs.getX() && lhs.getY() == rhs.getY() && lhs.getZ() == rhs.getZ();
}

template<typename T>
bool operator!=(const Vertex<T>& lhs, const Vertex<T>& rhs) { return !(lhs == rhs); }

// Define hash function for Vertex class
namespace std
{
    template<typename T>
    struct hash<Vertex<T>>
    {
        size_t operator()(const Vertex<T>& vertex) const
        {
            size_t hashValue = 17;
            hashValue = hashValue * 31 + hash<T>()(vertex.getX());
            hashValue = hashValue * 31 + hash<T>()(vertex.getY());
            hashValue = hashValue * 31 + hash<T>()(vertex.getZ());
            return hashValue;
        }
    };
}

#endif
