#include "SphereCollider.h"
#include "FileIO.h"
#include "Debug.h"

namespace UltraEd
{
    SphereCollider::SphereCollider() :
        m_radius(1),
        m_originalRadius(1)
    {
        m_type = ColliderType::Sphere;
    }

    SphereCollider::SphereCollider(const std::vector<Vertex> &vertices) : SphereCollider()
    {
        // Compute optimal center and radius (sphere)
        RitterIterative(m_center, m_radius, vertices);

        Build();

        m_originalCenter = m_center;
        m_originalRadius = m_radius;
    }

    void SphereCollider::Build()
    {
        const int segments = 32;
        const float sample = (2 * D3DX_PI) / segments;

        m_vertices.clear();

        // Horizontal ring
        for (int i = 0; i < segments; i++)
        {
            Vertex v1;
            v1.position = D3DXVECTOR3(sinf(sample * i) * m_radius, 0, cosf(sample * i) * m_radius) + m_center;
            m_vertices.push_back(v1);

            Vertex v2;
            v2.position = D3DXVECTOR3(sinf(sample * (i + 1)) * m_radius, 0, cosf(sample * (i + 1)) * m_radius) + m_center;
            m_vertices.push_back(v2);
        }

        // Vertical ring
        for (int i = 0; i < segments; i++)
        {
            Vertex v1;
            v1.position = D3DXVECTOR3(0, sinf(sample * i) * m_radius, cosf(sample * i) * m_radius) + m_center;
            m_vertices.push_back(v1);

            Vertex v2;
            v2.position = D3DXVECTOR3(0, sinf(sample * (i + 1)) * m_radius, cosf(sample * (i + 1)) * m_radius) + m_center;
            m_vertices.push_back(v2);
        }
    }

    void SphereCollider::Update(D3DXMATRIX &mat)
    {
        D3DXVECTOR3 scale, trans;
        D3DXQUATERNION rot;
        D3DXMatrixDecompose(&scale, &rot, &trans, &mat);

        // Scale the calculated radius using the largest scale value of the actor.
        std::vector<float> scaleComps { fabs(scale.x), fabs(scale.y), fabs(scale.z) };
        m_radius = m_originalRadius * (*std::max_element(scaleComps.begin(), scaleComps.end()));

        // Clear translation from matrix.
        mat._41 = mat._42 = mat._43 = 0;
        D3DXVec3TransformCoord(&m_center, &m_originalCenter, &mat);

        Build();
        Release();
    }

    void SphereCollider::SphereFromDistPoints(D3DXVECTOR3 &center, FLOAT &radius, const std::vector<Vertex> &vertices)
    {
        D3DXVECTOR3 min, max;
        MostDistantAABBPoints(min, max, vertices);

        center = (min + max) * 0.5f;
        radius = sqrtf(D3DXVec3Dot(&(max - center), &(max - center)));
    }

    void SphereCollider::AdjustSphere(D3DXVECTOR3 &center, FLOAT &radius, const Vertex &vertex)
    {
        D3DXVECTOR3 dist = vertex.position - center;
        FLOAT dist2 = D3DXVec3Dot(&dist, &dist);

        if (dist2 > radius * radius)
        {
            FLOAT dn = sqrtf(dist2);
            FLOAT newRadius = (radius + dn) * 0.5f;
            FLOAT k = (newRadius - radius) / dn;
            radius = newRadius;
            center += dist * k;
        }
    }

    void SphereCollider::RitterSphere(D3DXVECTOR3 &center, FLOAT &radius, const std::vector<Vertex> &vertices)
    {
        SphereFromDistPoints(center, radius, vertices);

        for (size_t i = 0; i < vertices.size(); i++)
        {
            AdjustSphere(center, radius, vertices[i]);
        }
    }

    void SphereCollider::RitterIterative(D3DXVECTOR3 &center, FLOAT &radius, const std::vector<Vertex> &vertices)
    {
        const int iterations = 12;
        const int vertexCount = static_cast<int>(vertices.size());

        RitterSphere(center, radius, vertices);

        D3DXVECTOR3 newCenter = center;
        FLOAT newRadius = radius;
        boost::random::mt19937 engine;

        for (int k = 0; k < iterations; k++)
        {
            newRadius = newRadius * 0.95f;

            for (int i = 0; i < vertexCount; i++)
            {
                boost::random::uniform_int_distribution<> range(i, vertexCount - 1);

                AdjustSphere(newCenter, newRadius, vertices[range(engine)]);
            }

            if (newRadius < radius)
            {
                center = newCenter;
                radius = newRadius;
            }
        }
    }

    nlohmann::json SphereCollider::Save()
    {
        auto collider = Collider::Save();
        collider.update({
            { "radius", m_radius },
            { "originalRadius", m_originalRadius }
        });
        return collider;
    }

    void SphereCollider::Load(const nlohmann::json &root)
    {
        Collider::Load(root);

        m_radius = root["radius"];
        m_originalRadius = root["originalRadius"];

        Build();
    }
}
