/*****************************************************************************/

/* D2K: A Doom Source Port for the 21st Century                              */
/*                                                                           */
/* Copyright (C) 2014: See COPYRIGHT file                                    */
/*                                                                           */
/* This file is part of D2K.                                                 */
/*                                                                           */
/* D2K is free software: you can redistribute it and/or modify it under the  */
/* terms of the GNU General Public License as published by the Free Software */
/* Foundation, either version 2 of the License, or (at your option) any      */
/* later version.                                                            */
/*                                                                           */
/* D2K is distributed in the hope that it will be useful, but WITHOUT ANY    */
/* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS */
/* FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more    */
/* details.                                                                  */
/*                                                                           */
/* You should have received a copy of the GNU General Public License along   */
/* with D2K.  If not, see <http://www.gnu.org/licenses/>.                    */
/*                                                                           */
/*****************************************************************************/

// e6y
void P_WalkTicker(void) {
  int strafe;
  int speed;
  int tspeed;
  int turnheld;
  int forward;
  int side;
  int angturn;
  subsector_t *subsec = NULL;

  if (!walkcamera.type || menuactive) {
    return;
  }

  strafe = gamekeydown[key_strafe]    ||
           mousebuttons[mousebstrafe] ||
           joybuttons[joybstrafe];
  speed = autorun || gamekeydown[key_speed] || joybuttons[joybspeed]; // phares

  forward = side = 0;
  angturn = 0;
  turnheld = 0;

  // use two stage accelerative turning
  // on the keyboard and joystick
  if (joyxmove < 0 || joyxmove > 0 || gamekeydown[key_right]
                                   || gamekeydown[key_left]) {
    turnheld++;
  }
  else {
    turnheld = 0;
  }

  if (turnheld < SLOWTURNTICS) {
    tspeed = 0; // slow turn
  }
  else {
    tspeed = speed; // phares
  }

  // let movement keys cancel each other out
  if (strafe) {
    if (gamekeydown[key_right]) {
      side += sidemove[speed];
    }
    if (gamekeydown[key_left]) {
      side -= sidemove[speed];
    }
    if (joyxmove > 0) {
      side += sidemove[speed];
    }
    if (joyxmove < 0) {
      side -= sidemove[speed];
    }
  }
  else {
    if (gamekeydown[key_right]) {
      angturn -= angleturn[tspeed];
    }
    if (gamekeydown[key_left]) {
      angturn += angleturn[tspeed];
    }
    if (joyxmove > 0) {
      angturn -= angleturn[tspeed];
    }
    if (joyxmove < 0) {
      angturn += angleturn[tspeed];
    }
  }

  if (gamekeydown[key_up]) {
    forward += forwardmove[speed];
  }
  if (gamekeydown[key_down]) {
    forward -= forwardmove[speed];
  }
  if (joyymove < 0) {
    forward += forwardmove[speed];
  }
  if (joyymove > 0) {
    forward -= forwardmove[speed];
  }
  if (gamekeydown[key_straferight]) {
    side += sidemove[speed];
  }
  if (gamekeydown[key_strafeleft]) {
    side -= sidemove[speed];
  }

  // mouse
  if (mousebuttons[mousebforward]) {
    forward += forwardmove[speed];
  }

  forward += mousey;

  if (strafe) {
    side += mousex / 4;/* mead  Don't want to strafe as fast as turns.*/
  }
  else {
    angturn -= mousex; /* mead now have enough dynamic range 2-10-00 */
  }

  walkcamera.angle += ((angturn / 8) << ANGLETOFINESHIFT);

  if (GetMouseLook()) {
    walkcamera.pitch += ((mlooky / 8) << ANGLETOFINESHIFT);
    CheckPitch((signed int *)&walkcamera.pitch);
  }

  if (gamekeydown[key_fire] ||
    mousebuttons[mousebfire] ||
    joybuttons[joybfire]) {
    player_t *consoleplayer = P_GetConsolePlayer();

    walkcamera.x = consoleplayer->mo->x;
    walkcamera.y = consoleplayer->mo->y;
    walkcamera.angle = consoleplayer->mo->angle;
    walkcamera.pitch = consoleplayer->mo->pitch;
  }

  if (forward > MAXPLMOVE) {
    forward = MAXPLMOVE;
  }
  else if (forward < -MAXPLMOVE) {
    forward = -MAXPLMOVE;
  }
  if (side > MAXPLMOVE) {
    side = MAXPLMOVE;
  }
  else if (side < -MAXPLMOVE) {
    side = -MAXPLMOVE;
  }

  // moving forward
  walkcamera.x += FixedMul(
    (ORIG_FRICTION / 4) * forward,
    finecosine[walkcamera.angle >> ANGLETOFINESHIFT]
    );
  walkcamera.y += FixedMul(
    (ORIG_FRICTION / 4) * forward,
    finesine[walkcamera.angle >> ANGLETOFINESHIFT]
    );

  // strafing
  walkcamera.x += FixedMul(
    (ORIG_FRICTION / 6) * side,
    finecosine[(walkcamera.angle - ANG90) >> ANGLETOFINESHIFT]
    );
  walkcamera.y += FixedMul(
    (ORIG_FRICTION / 6) * side,
    finesine[(walkcamera.angle - ANG90) >> ANGLETOFINESHIFT]
    );

  subsec = R_PointInSubsector(walkcamera.x, walkcamera.y);
  walkcamera.z = subsec->sector->floorheight + 41 * FRACUNIT;

  mousex = mousey = 0;
}

void P_ResetWalkcam(void) {
  if (walkcamera.type) {
    walkcamera.PrevX = walkcamera.x;
    walkcamera.PrevY = walkcamera.y;
    walkcamera.PrevZ = walkcamera.z;
    walkcamera.PrevAngle = walkcamera.angle;
    walkcamera.PrevPitch = walkcamera.pitch;
  }
}

void P_SyncWalkcam(bool sync_coords, bool sync_sight) {
  if (!walkcamera.type) {
    return;
  }

  if (P_GetDisplayPlayer()->mo) {
    if (sync_sight) {
      walkcamera.angle = P_GetDisplayPlayer()->mo->angle;
      walkcamera.pitch = P_GetDisplayPlayer()->mo->pitch;
    }

    if (sync_coords) {
      walkcamera.x = P_GetDisplayPlayer()->mo->x;
      walkcamera.y = P_GetDisplayPlayer()->mo->y;
    }
  }
}

/* vi: set et ts=2 sw=2: */
