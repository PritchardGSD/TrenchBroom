/*
 Copyright (C) 2010-2012 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "RotateObjectsCommand.h"

#include "Model/Brush.h"
#include "Model/Entity.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        bool RotateObjectsCommand::performDo() {
            float angle = m_angle;
            if (!m_clockwise)
                angle *= -1.0f;
            
            if (angle > 0.0f) {
                while (angle - 2.0f * Math::Pi >= 0.0f)
                    angle -= 2.0f * Math::Pi;
            } else if (angle < 0.0f) {
                while (angle + 2.0f * Math::Pi <= 0.0f)
                    angle += 2.0f * Math::Pi;
            }
            
            if (angle == 0.0f)
                return false;
            
            if (angle < 0.0f)
                angle = 2.0f * Math::Pi - angle;
            
            assert(angle > 0.0f);
            
            // if we are rotating about one of the coordinate system axes, we can get a more precise result by rotating
            // by 90 degrees as often as possible
            if (m_axis.equals(Vec3f::PosX) || m_axis.equals(Vec3f::PosY) || m_axis.equals(Vec3f::PosZ) ||
                m_axis.equals(Vec3f::NegX) || m_axis.equals(Vec3f::NegY) || m_axis.equals(Vec3f::NegZ)) {
                unsigned int quarters = 2.0f * m_angle / Math::Pi;
                
                if (quarters > 0) {
                    angle = angle - quarters * Math::Pi / 2.0f;
                    Axis::Type component = m_axis.firstComponent();
                    
                    for (unsigned int i = 0; i < quarters; i++) {
                        Model::EntityList::const_iterator entityIt, entityEnd;
                        for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                            Model::Entity& entity = **entityIt;
                            entity.rotate90(component, m_center, m_clockwise, m_lockTextures);
                            document().updateEntity(entity);
                        }
                        
                        Model::BrushList::const_iterator brushIt, brushEnd;
                        for (brushIt = m_brushes.begin(), brushEnd = m_brushes.end(); brushIt != brushEnd; ++brushIt) {
                            Model::Brush& brush = **brushIt;
                            brush.rotate90(component, m_center, m_clockwise, m_lockTextures);
                            document().updateBrush(brush);
                        }
                    }
                }
            }

            Quat rotation(angle, m_axis);
            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = m_entities.begin(), entityEnd = m_entities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& entity = **entityIt;
                entity.rotate(rotation, m_center, m_lockTextures);
                document().updateEntity(entity);
            }
            
            Model::BrushList::const_iterator brushIt, brushEnd;
            for (brushIt = m_brushes.begin(), brushEnd = m_brushes.end(); brushIt != brushEnd; ++brushIt) {
                Model::Brush& brush = **brushIt;
                brush.rotate(rotation, m_center, m_lockTextures);
                document().updateBrush(brush);
            }
            
            return true;
        }
        
        bool RotateObjectsCommand::performUndo() {
            m_clockwise = !m_clockwise;
            performDo();
            m_clockwise = !m_clockwise;
            
            return true;
        }
        
        RotateObjectsCommand::RotateObjectsCommand(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes, const wxString& name, const Vec3f& axis, float angle, bool clockwise, const Vec3f& center, bool lockTextures) :
        DocumentCommand(RotateObjects, document, true, name),
        m_entities(entities),
        m_brushes(brushes),
        m_axis(axis),
        m_angle(angle),
        m_clockwise(clockwise),
        m_center(center),
        m_lockTextures(lockTextures) {}

        RotateObjectsCommand* RotateObjectsCommand::rotate(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes, const Vec3f& axis, float angle, bool clockwise, const Vec3f& center, bool lockTextures) {
            wxString commandName = Command::makeObjectActionName(wxT("Rotate"), entities, brushes);
            return new RotateObjectsCommand(document, entities, brushes, commandName, axis, angle, clockwise, center, lockTextures);
        }
    }
}