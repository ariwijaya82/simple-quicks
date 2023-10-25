#include "render_area.hpp"

#include <iostream>
#include <cmath>

using namespace std;

RenderArea::RenderArea(GlobalData* global, QWidget* parent) : QWidget(parent), global(global) {
  setFixedSize(940 ,640);
  setMouseTracking(true);

  timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &RenderArea::render);
  
  generator = new PathGenerator(global);
}

void RenderArea::render() {
  update();
}

void RenderArea::setMode() {
  switch (global->mode) {
    case 0: {
      global->isGenerate = false;
      global->isStart = false;
      global->isStatic = false;

      global->astar_path.clear();
      global->bezier_path.clear();
      global->visited_node.clear();

      global->updateGlobal();
      global->updatePosition();
      global->updateObstacles();
      global->updateTargetPosition();
      break;
    }
    
    case 1: {
      global->isGenerate = false;
      global->astar_path.clear();
      global->obstacles.clear();
      global->obstacles.push_back(vector<Vec>());
      generator->openList.clear();
      generator->closeList.clear();
      break;
    }

    case 2: {
      global->isGenerate = false;
      global->astar_path.clear();
      global->bezier_path.clear();
      global->control_points.clear();
      global->modified_astar_path.clear();
      break;
    }
  }
  update();
}

void RenderArea::setAnimate(bool value) {
  if (value) timer->start(50);
  else timer->stop();
}

void RenderArea::setGeneratePath(bool value) {
  if (value) {
    generator->generatePath();
    generator->generateSmoothPath(100);
  } else {
    global->astar_path.clear();
    global->bezier_path.clear();
    global->visited_node.clear();
  }
}

bool RenderArea::setPathNext(bool val) {
  if (val) {
    if (!global->isGenerate) {
      global->isGenerate = true;

      generator->current = nullptr;
      generator->openList.clear();
      generator->closeList.clear();
      
      if ((global->robot - global->ball).len() < global->robot_radius) {
        global->astar_path = vector<Vec>{global->robot, global->ball};
        return true;
      }

      generator->openList.push_back(new Node(global->robot));
    }
    if (generator->openList.empty()) return true;
    generator->astar_find_next_node();
    if (generator->current->coordinate == global->ball) return true;
    generator->astar_find_neighbors();
  } else {
    global->isGenerate = false;
    generator->openList.clear();
    generator->closeList.clear();
  }
  return false;
}

void RenderArea::setModifiedPath(bool val) {
  if (val) {
    generator->modified_path(false);
  } else {
    global->astar_path.clear();
    global->modified_astar_path.clear();
    global->bezier_path.clear();
    global->control_points.clear();
  }
}

void RenderArea::handlePanelChange(string widget, int value) {
  if (widget == "staticCheck") {
    if (value == 0) {
      global->isStatic = false;
    } else if (value == 2) {
      global->isStatic = true;
    }
  } else if (widget == "bezierSlider") {
    generator->getBezierPoints(100, value);
  } else if (widget == "pathCombo") {
    global->path_number = value;
    global->updatePosition();
    global->updateObstacles();
    global->updateTargetPosition();
    if (global->isGenerate) setGeneratePath(true);
  } else if (widget == "heuristicCombo") {
    global->heuristic_type = value;
    if (global->isGenerate) setGeneratePath(true);
  } else if (widget == "nodeSpin") {
    global->node_distance = value;
    if (global->mode == 0) {
      global->updateObstacles();
      if (global->isGenerate) setGeneratePath(true);
    }
  } else if (widget == "radiusSpin") {
    global->robot_radius = value * 2;
    global->updateObstacles();
    if (global->isGenerate) setGeneratePath(true);
  } else if (widget == "bezierSpin") {
    global->bezier_curvature = value;
    if (global->mode == 0 && global->isGenerate) setGeneratePath(true);
    if (global->mode == 2) {
      setModifiedPath(true);
    }
  }
  update();
}

void RenderArea::paintEvent(QPaintEvent* paintEvent) {
  QPainter painter(this);
  switch (global->mode) {
    case 0: {
      painter.fillRect(rect(), Qt::green);
      painter.setPen(Qt::white);
      painter.drawRect(20, 20, 900, 600);
      painter.drawRect(20, 70, 200, 500);
      painter.drawRect(20, 170, 100, 300);
      painter.drawRect(720, 70, 200, 500);
      painter.drawRect(820, 170, 100, 300);
      painter.drawEllipse(QPoint(470, 320), 100, 100);
      painter.drawLine(470, 20, 470, 620);

      painter.setPen(Qt::yellow);
      painter.setBrush(Qt::yellow);
      for (int x = global->node_distance; x < global->screen_width; x += global->node_distance) {
        for (int y = global->node_distance; y < global->screen_height; y += global->node_distance) {
          painter.drawEllipse(transformPoint(Vec(x, y)), 2, 2);
        }
      }

      painter.setPen(Qt::red);
      painter.setBrush(Qt::red);
      for (auto& data : global->obstacles) {
        for (auto& point : data) {
          painter.drawEllipse(transformPoint(point), 2, 2);
        }
      }

      painter.setPen(Qt::black);
      painter.setBrush(Qt::black);
      painter.drawEllipse(transformPoint(global->robot), 8, 8);
      painter.setBrush(Qt::NoBrush);
      painter.drawEllipse(transformPoint(global->robot), (int)global->robot_radius/2, (int)global->robot_radius/2);
      painter.setPen(Qt::white);
      painter.setBrush(Qt::white);
      painter.drawEllipse(transformPoint(global->ball), 8, 8);
      painter.setBrush(Qt::NoBrush);
      painter.drawEllipse(transformPoint(global->ball), (int)global->robot_radius/2, (int)global->robot_radius/2);
      for (size_t i = 0; i < global->enemies.size(); i++) {
        painter.setPen(Qt::red);
        painter.setBrush(Qt::red);
        if (isSelectEnemy && i == index_enemy_select) {
          painter.setPen(Qt::darkRed);
          painter.setBrush(Qt::darkRed);  
        }
        painter.drawEllipse(transformPoint(global->enemies[i]), 8, 8);
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(transformPoint(global->enemies[i]), (int)global->robot_radius/2, (int)global->robot_radius/2);
        painter.setPen(QPen(Qt::red, 1, Qt::DashLine));
        painter.drawEllipse(transformPoint(global->enemies[i]), (int)global->robot_radius, (int)global->robot_radius);
      }

      if (!global->isStatic) {
        painter.setPen(Qt::darkBlue);
        painter.setBrush(Qt::darkBlue);
        for (size_t i = 0; i < global->target_position.size(); i++) {
          for (size_t j = 0; j < global->target_position[i].size(); j++) {
            painter.drawText(transformPoint(global->target_position[i][j] - Vec(4,4)), "x");
            painter.setPen(QPen(Qt::darkBlue, 1, Qt::DashLine));
            if (j == 0) painter.drawLine(transformPoint(global->enemies[i]), transformPoint(global->target_position[i][j]));
            else painter.drawLine(transformPoint(global->target_position[i][j-1]), transformPoint(global->target_position[i][j]));
          } 
        }
      }

      painter.setPen(Qt::gray);
      painter.setBrush(Qt::gray);
      for (size_t i = 0; i < global->visited_node.size(); i++) {
          painter.drawEllipse(transformPoint(global->visited_node[i]), 2, 2);
      }
      painter.setPen(Qt::blue);
      painter.setBrush(Qt::blue);
      for (size_t i = 0; i < global->astar_path.size(); i++) {
          painter.drawEllipse(transformPoint(global->astar_path[i]), 2, 2);
          if (i != 0) {
              painter.drawLine(transformPoint(global->astar_path[i]), transformPoint(global->astar_path[i-1]));
          }
      }
      painter.setPen(Qt::magenta);
      painter.setBrush(Qt::magenta);
      for (size_t i = 1; i < global->bezier_path.size(); i++) {
          painter.drawLine(transformPoint(global->bezier_path[i]), transformPoint(global->bezier_path[i-1]));
      }
      break;
    }

    case 1: {
      painter.fillRect(rect(), Qt::green);

      painter.setPen(Qt::yellow);
      painter.setBrush(Qt::yellow);
      for (int x = global->node_distance; x < global->screen_width; x += global->node_distance) {
        for (int y = global->node_distance; y < global->screen_height; y += global->node_distance) {
          painter.drawEllipse(transformPoint(Vec(x, y)), 4, 4);
        }
      }

      painter.setPen(Qt::red);
      painter.setBrush(Qt::red);
      for (auto& data : global->obstacles) {
        for (auto& point : data) {
          painter.drawEllipse(transformPoint(point), 4, 4);
        }
      }

      painter.setPen(Qt::black);
      painter.setBrush(Qt::black);
      painter.drawEllipse(transformPoint(global->robot), 8, 8);
      painter.setPen(Qt::white);
      painter.setBrush(Qt::white);
      painter.drawEllipse(transformPoint(global->ball), 8, 8);

      painter.drawText(880, 615, "x: " + QString::number(mouseCurr.x()));
      painter.drawText(880, 630, "y: " + QString::number(mouseCurr.y()));
      
      if (isMouseClicked && !isMouseInStart && !isMouseInTarget && !isMouseInNode) {
        painter.setPen(Qt::red);
        painter.setBrush(Qt::NoBrush);
        QPoint delta = mouseCurr - mouseStart;
        painter.drawRect(mouseStart.x(), mouseStart.y(), delta.x(), delta.y());
      }
      for (auto &data : generator->openList) {
        painter.setPen(Qt::white);
        QPoint parent, child = transformPoint(data->coordinate);
        if (data->parent != nullptr) {
          parent = transformPoint(data->parent->coordinate);
          painter.drawLine(parent, child);
        }
      }
      for (auto &data : generator->closeList) {
        painter.setPen(Qt::black);
        QPoint parent, child = transformPoint(data->coordinate);
        if (data->parent != nullptr) {
          parent = transformPoint(data->parent->coordinate);
          painter.drawLine(parent, child);
        }
      }
      break;
    }
    
    case 2: {
      painter.fillRect(rect(), Qt::green);

      painter.setPen(Qt::yellow);
      painter.setBrush(Qt::yellow);
      for (int x = global->node_distance; x < global->screen_width; x += global->node_distance) {
        for (int y = global->node_distance; y < global->screen_height; y += global->node_distance) {
          painter.drawEllipse(transformPoint(Vec(x, y)), 4, 4);
        }
      }

      painter.setPen(Qt::blue);
      painter.setBrush(Qt::blue);
      for (size_t i = 0; i < global->astar_path.size(); i++) {
          painter.drawEllipse(transformPoint(global->astar_path[i]), 4, 4);
          if (i != 0) {
              painter.drawLine(transformPoint(global->astar_path[i]), transformPoint(global->astar_path[i-1]));
          }
      }

      painter.setPen(Qt::magenta);
      painter.setBrush(Qt::magenta);
      for (size_t i = 1; i < global->bezier_path.size(); i++) {
          painter.drawLine(transformPoint(global->bezier_path[i]), transformPoint(global->bezier_path[i-1]));
      }
      if (global->control_points.size() > 1) {
        for (size_t i = global->control_points.size()-1; i > global->control_points.size()-global->modified_astar_path.size(); i--) {
          for (size_t j = 0; j < global->control_points[i].size(); j++) {
            painter.drawEllipse(transformPoint(global->control_points[i][j]), 2, 2);
            if (j != 0) {
              painter.drawLine(transformPoint(global->control_points[i][j]), transformPoint(global->control_points[i][j-1]));
            }
          }
        }
      } 
      break;
    }
  }
}

QPoint RenderArea::transformPoint(Vec point) {
  return QPoint(point.x + global->screen_padding, global->screen_height + global->screen_padding - point.y);
}

Vec RenderArea::transformPoint(QPoint point) {
  return Vec(point.x() - global->screen_padding, global->screen_height + global->screen_padding - point.y());
}

void RenderArea::mousePressEvent(QMouseEvent* event) {
  if (!isMouseClicked) {
    isMouseClicked = true;
    mouseStart = event->pos();
    switch (global->mode) {
      case 0: {
        isMouseInStart = false;
        isMouseInTarget = false;
        isMouseInTarPos = false;
        isMouseInEnemy = false;

        index_enemy = -1;
        index_i = -1;
        index_j = -1;

        QPoint delta;
        for (size_t i = 0; i < global->target_position.size(); i++) {
          for (size_t j = 0; j < global->target_position[i].size(); j++) {
            delta = transformPoint(global->target_position[i][j]) - event->pos();
            if (abs(delta.x()) < 10 && abs(delta.y()) < 10) {
              isMouseInTarPos = true;
              index_i = i;
              index_j = j;
              mouseOffset = delta;
            }
          } 
        }

        if (global->mode == 0) {
          for (size_t i = 0; i < global->enemies.size(); i++) {
            delta = transformPoint(global->enemies[i]) - event->pos();
            if (abs(delta.x()) < 10 && abs(delta.y()) < 10) {
              isMouseInEnemy = true;
              index_enemy = i;
              mouseOffset = delta;
              break;
            }
          }

          delta = transformPoint(global->robot) - event->pos();
          if (abs(delta.x()) < 10 && abs(delta.y()) < 10) {
            isMouseInStart = true;
            mouseOffset = delta;
            break;
          }
          delta = transformPoint(global->ball) - event->pos();
          if (abs(delta.x()) < 10 && abs(delta.y()) < 10) {
            isMouseInTarget = true;
            mouseOffset = delta;
            break;
          }
        }
        break;
      }
    
      case 1: {
        isMouseInStart = false;
        isMouseInTarget = false;
        isMouseInNode = false;

        index_i = -1;
        index_j = -1;

        QPoint delta = transformPoint(global->robot) - event->pos();
        if (abs(delta.x()) < 10 && abs(delta.y()) < 10) {
          isMouseInStart = true;
          mouseOffset = delta;
          break;
        }
        delta = transformPoint(global->ball) - event->pos();
        if (abs(delta.x()) < 10 && abs(delta.y()) < 10) {
          isMouseInTarget = true;
          mouseOffset = delta;
          break;
        }

        for (int x = global->node_distance; x < global->screen_width; x += global->node_distance) {
          for (int y = global->node_distance; y < global->screen_height; y += global->node_distance) {
            delta = transformPoint(Vec(x, y)) - event->pos();
            if (abs(delta.x()) < 10 && abs(delta.y()) < 10) {
              isMouseInNode = true;
              mouseOffset = delta;
              index_i = x;
              index_j = y;
              break;
            }
          }
        }
      }
    
      case 2: {
        isMouseInNode = false;
        index_i = -1;
        index_j = -1;

        for (int x = global->node_distance; x < global->screen_width; x += global->node_distance) {
          for (int y = global->node_distance; y < global->screen_height; y += global->node_distance) {
            QPoint delta = transformPoint(Vec(x, y)) - event->pos();
            if (abs(delta.x()) < 10 && abs(delta.y()) < 10) {
              isMouseInNode = true;
              mouseOffset = delta;
              index_i = x;
              index_j = y;
              break;
            }
          }
        }
      }
    }
  }
  update();
}

void RenderArea::mouseMoveEvent(QMouseEvent* event) {
  mouseCurr = event->pos();
  if (isMouseClicked) {
    switch (global->mode) {
      case 0: {
        if (isMouseInTarPos) {
          global->target_position[index_i][index_j] = transformPoint(event->pos() + mouseOffset);
        } else if (isMouseInEnemy) {
          global->enemies[index_enemy] = transformPoint(event->pos() + mouseOffset);
          global->updateObstacles();
        } else if (isMouseInStart) {
          global->robot = transformPoint(event->pos() + mouseOffset);
        } else if (isMouseInTarget) {
          global->ball = transformPoint(event->pos() + mouseOffset);
        }
        break;
      }

      case 1: {
        if (isMouseInStart) {
          global->robot = transformPoint(event->pos() + mouseOffset);
        } else if (isMouseInTarget) {
          global->ball = transformPoint(event->pos() + mouseOffset);
        }
        break;
      }
    }
  }
  update();
}

void RenderArea::mouseReleaseEvent(QMouseEvent* event) {
  isMouseClicked = false;
  mouseRelease = event->pos();
  switch (global->mode) {
    case 0: {
      QPoint delta = mouseStart - mouseRelease;
      if (isSelectEnemy) {
        if (index_enemy_select == index_enemy) {
          isSelectEnemy = false;
          index_enemy_select = -1;
        } else if (index_i == index_enemy_select && 
          index_j == global->target_position[index_i].size()-1) {
          global->target_position[index_i].pop_back();
        } else {
          global->target_position[index_enemy_select].push_back(transformPoint(mouseRelease));
        }
      }
      else if (abs(delta.x()) < 10 && abs(delta.y()) < 10) {
        if (isMouseInEnemy) {
          isSelectEnemy = true;
          index_enemy_select = index_enemy;
        }
      } 
      break;
    }

    case 1: {
      if (!isMouseInStart && !isMouseInTarget && !isMouseInNode) {
        for (int i = 0; i < global->screen_width; i += global->node_distance) {
          for (int j = 0; j < global->screen_width; j += global->node_distance) {
            QPoint temp = transformPoint(Vec(i, j));
            if (temp.x() > mouseStart.x() && temp.x() < mouseRelease.x() &&
            temp.y() > mouseStart.y() && temp.y() < mouseRelease.y()) {
              global->obstacles[0].push_back(Vec(i, j));
            }
          } 
        }
      } else if (isMouseInNode) {
        global->obstacles[0].push_back(Vec(index_i, index_j));
      }
      break;
    }

    case 2: {
      if (isMouseInNode) {
        bool isFound = false;
        for (auto &data : global->astar_path) {
          if (data == Vec(index_i, index_j)) {
            isFound = true;
            break;
          } 
        }
        if (!isFound) global->astar_path.push_back(Vec(index_i, index_j));
      }
      break;
    }
  }
  update();
}